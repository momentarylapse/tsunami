/*
 * Clipboard.cpp
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#include "Clipboard.h"
#include "../View/AudioView.h"
#include "../Action/Track/Sample/ActionTrackPasteAsSample.h"
#include "../Action/Track/Buffer/ActionTrackEditBuffer.h"
#include "../Session.h"
#include "../Data/base.h"
#include "../Data/Song.h"
#include "../Data/Track.h"
#include "../Data/TrackLayer.h"
#include "../Data/Sample.h"
#include "../Data/SongSelection.h"
#include <assert.h>

Clipboard::Clipboard()
{
	temp = new Song(Session::GLOBAL, DEFAULT_SAMPLE_RATE);
}

Clipboard::~Clipboard()
{
	delete(temp);
}

void Clipboard::clear()
{
	if (temp->tracks.num > 0){
		temp->reset();
		notify();
	}
	ref_uid.clear();
}

void Clipboard::append_track(TrackLayer *l, AudioView *view, int offset)
{
	if (l->type == SignalType::BEATS)
		return;

	TrackLayer *ll = temp->add_track(l->type)->layers[0];

	if (l->type == SignalType::AUDIO){
		AudioBuffer buf;
		ll->buffers.add(buf);
		l->read_buffers(ll->buffers[0], view->sel.range, false);
	}else if (l->type == SignalType::MIDI){
		ll->midi = l->midi.get_notes_by_selection(view->sel);
		ll->midi.samples = view->sel.range.length;
		ll->midi.sanify(view->sel.range);
		for (MidiNote *n: ll->midi)
			n->range.offset -= offset;
	}

	ref_uid.add(-1);
}

static int find_offset(AudioView *view)
{
	if (!view->sel.range.empty())
		return view->sel.range.offset;

	int offset = view->song->range().end();
	for (Track *t: view->song->tracks)
		for (TrackLayer *l: t->layers)
			if (view->sel.has(l))
				for (MidiNote *n: l->midi)
					if (view->sel.has(n))
						offset = min(offset, n->range.offset);

	return offset;
}

#include "../Data/Rhythm/Bar.h"

void Clipboard::copy(AudioView *view)
{
	if (!can_copy(view))
		return;
	clear();

	Song *s = view->song;

	temp->sample_rate = s->sample_rate;

	int offset = find_offset(view);

	for (Bar *b: s->bars)
		if (view->sel.has(b)){
			if (temp->bars.num == 0 and b->offset > offset)
				temp->bars.add(new Bar(b->offset - offset, 0, 0));
			temp->bars.add(new Bar(b->length, b->num_beats, b->num_sub_beats));
		}

	for (Track *t: s->tracks)
		for (TrackLayer *l: t->layers)
			if (view->sel.has(l))
				append_track(l, view, offset);

	notify();
}

void Clipboard::paste_track(int source_index, TrackLayer *target, AudioView *view)
{
	Song *s = target->track->song;
	TrackLayer *source = temp->tracks[source_index]->layers[0];

	if (target->type == SignalType::AUDIO){
		Range r = Range(view->sel.range.start(), source->buffers[0].length);
		AudioBuffer buf;
		target->get_buffers(buf, r);
		Action *a = new ActionTrackEditBuffer(target, r);
		buf.set(source->buffers[0], 0, 1.0f);
		s->execute(a);
	}else if (target->type == SignalType::MIDI){
		for (MidiNote *n: source->midi){
			MidiNote *nn = n->copy();
			nn->range.offset += view->sel.range.start();
			target->add_midi_note(nn);
		}
	}
}

void Clipboard::paste_track_as_samples(int source_index, TrackLayer *target, AudioView *view)
{
	Song *s = target->track->song;
	Track *source = temp->tracks[source_index];

	Sample* ref = s->get_sample_by_uid(ref_uid[source_index]);
	if (ref){
		target->add_sample_ref(view->sel.range.start(), ref);
	}else{
		if (target->type == SignalType::AUDIO){MidiNoteBuffer midi;
			Sample *sample = (Sample*)s->execute(new ActionTrackPasteAsSample(target, view->sel.range.start(), source->layers[0]->buffers[0], true));
			ref_uid[source_index] = sample->uid;
		}else if (target->type == SignalType::MIDI){
			Sample *sample = (Sample*)s->execute(new ActionTrackPasteAsSample(target, view->sel.range.start(), source->layers[0]->midi, true));
			ref_uid[source_index] = sample->uid;
		}
	}
}

bool Clipboard::test_compatibility(AudioView *view)
{
	Array<string> temp_type, dest_type;
	for (Track *t: view->song->tracks){
		for (TrackLayer *l: t->layers){
			if (!view->sel.has(l))
				continue;
			if (l->type == SignalType::BEATS)
				continue;
			dest_type.add(signal_type_name(l->type));
		}
	}

	for (Track *t: temp->tracks)
		temp_type.add(signal_type_name(t->type));

	if (dest_type.num != temp->tracks.num){
		view->session->e(format(_("%d tracks selected for pasting (ignoring the metronome), but %d tracks in clipboard"), dest_type.num, temp->tracks.num));
		return false;
	}
	string t1 = "[" + implode(temp_type, ", ") + "]";
	string t2 = "[" + implode(dest_type, ", ") + "]";
	if (t1 != t2){
		view->session->e(format(_("Track types in clipboard (%s) don't match those you want to paste into (%s)"), t1.c_str(), t2.c_str()));
		return false;
	}
	return true;
}

void Clipboard::paste(AudioView *view)
{
	if (!has_data())
		return;
	Song *s = view->song;

	if (!test_compatibility(view))
		return;


	s->begin_action_group();

	int ti = 0;
	for (Track *t: s->tracks){
		for (TrackLayer *l: t->layers){
			if (!view->sel.has(l))
				continue;
			if (t->type == SignalType::BEATS)
				continue;

			paste_track(ti, l, view);
			ti ++;
		}
	}
	s->end_action_group();
}

void Clipboard::paste_as_samples(AudioView *view)
{
	if (!has_data())
		return;
	Song *s = view->song;

	if (!test_compatibility(view))
		return;


	s->begin_action_group();

	int ti = 0;
	for (Track *t: s->tracks){
		for (TrackLayer *l: t->layers){
			if (!view->sel.has(l))
				continue;
			if (t->type == SignalType::BEATS)
				continue;

			paste_track_as_samples(ti, l, view);
			ti ++;
		}
	}
	s->end_action_group();
}

void Clipboard::paste_with_time(AudioView *view)
{
	if (!has_data())
		return;
	Song *s = view->song;
	s->begin_action_group();

	int offset = view->sel.range.offset;
	int index = 0;
	foreachi (Bar *b, s->bars, i)
		if (b->offset >= offset){
			index = i;
			break;
		}

	//if (temp->bars.num )
	for (Bar *b: temp->bars)
		s->add_bar(index ++, b->bpm(s->sample_rate), b->num_beats, b->num_sub_beats, Bar::EditMode::INSERT_SILENCE);

	paste(view);

	s->end_action_group();
}

bool Clipboard::has_data()
{
	return (temp->tracks.num > 0) or (temp->bars.num > 0);
}

bool Clipboard::can_copy(AudioView *view)
{
	if (!view->sel.range.empty())
		return true;
	if (view->sel.notes.num > 0)
		return true;
	//if (view->sel.samples.num > 0)
	//	return true;
	return false;
}

