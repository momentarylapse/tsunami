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
#include "../Data/Song.h"
#include "../Data/SongSelection.h"
#include <assert.h>

Clipboard::Clipboard()
{
	temp = new Song(Session::GLOBAL);
	temp->reset();
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

void Clipboard::append_track(TrackLayer *l, AudioView *view)
{
	if (l->type == Track::Type::TIME)
		return;

	TrackLayer *ll = temp->addTrack(l->type)->layers[0];

	if (l->type == Track::Type::AUDIO){
		ll->buffers.add(l->readBuffers(view->sel.range));
		ll->buffers[0].make_own();
	}else if (l->type == Track::Type::MIDI){
		ll->midi = l->midi.getNotesBySelection(view->sel);
		ll->midi.samples = view->sel.range.length;
		ll->midi.sanify(view->sel.range);
		for (MidiNote *n: ll->midi)
			n->range.offset -= view->sel.range.offset;
	}

	ref_uid.add(-1);
}

void Clipboard::copy(AudioView *view)
{
	if (!canCopy(view))
		return;
	clear();

	Song *s = view->song;

	temp->sample_rate = s->sample_rate;

	for (Track *t: s->tracks)
		for (TrackLayer *l: t->layers)
			if (view->sel.has(l))
				append_track(l, view);

	notify();
}

void Clipboard::paste_track(int source_index, TrackLayer *target, AudioView *view)
{
	Song *s = target->track->song;
	TrackLayer *source = temp->tracks[source_index]->layers[0];

	if (target->type == Track::Type::AUDIO){
		Range r = Range(view->sel.range.start(), source->buffers[0].length);
		AudioBuffer buf = target->getBuffers(r);
		Action *a = new ActionTrackEditBuffer(target, r);
		buf.set(source->buffers[0], 0, 1.0f);
		s->execute(a);
	}else if (target->type == Track::Type::MIDI){
		for (MidiNote *n: source->midi){
			MidiNote *nn = n->copy();
			nn->range.offset += view->sel.range.start();
			target->addMidiNote(nn);
		}
	}
}

void Clipboard::paste_track_as_samples(int source_index, TrackLayer *target, AudioView *view)
{
	/*Song *s = target->track->song;
	Track *source = temp->tracks[source_index];

	Sample* ref = s->get_sample_by_uid(ref_uid[source_index]);
	if (ref){
		target->addSampleRef(view->sel.range.start(), ref);
	}else{
		if (target->type == Track::Type::AUDIO){MidiNoteBuffer midi;
			Sample *sample = (Sample*)s->execute(new ActionTrackPasteAsSample(target, view->sel.range.start(), source->layers[0]->buffers[0], true));
			ref_uid[source_index] = sample->uid;
		}else if (target->type == Track::Type::MIDI){
			Sample *sample = (Sample*)s->execute(new ActionTrackPasteAsSample(target, view->sel.range.start(), source->midi, true));
			ref_uid[source_index] = sample->uid;
		}
	}*/

	// need tracks here...
	msg_write("TODO: paste as sample");
}

bool Clipboard::test_compatibility(AudioView *view)
{
	Array<string> temp_type, dest_type;
	for (Track *t: view->song->tracks){
		for (TrackLayer *l: t->layers){
			if (!view->sel.has(l))
				continue;
			if (l->type == Track::Type::TIME)
				continue;
			dest_type.add(track_type(l->type));
		}
	}

	for (Track *t: temp->tracks)
		temp_type.add(track_type(t->type));

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
	if (!hasData())
		return;
	Song *s = view->song;

	if (!test_compatibility(view))
		return;


	s->action_manager->beginActionGroup();

	int ti = 0;
	for (Track *t: s->tracks){
		for (TrackLayer *l: t->layers){
			if (!view->sel.has(l))
				continue;
			if (t->type == Track::Type::TIME)
				continue;

			paste_track(ti, l, view);
			ti ++;
		}
	}
	s->action_manager->endActionGroup();
}

void Clipboard::pasteAsSamples(AudioView *view)
{
	if (!hasData())
		return;
	Song *s = view->song;

	if (!test_compatibility(view))
		return;


	s->action_manager->beginActionGroup();

	int ti = 0;
	for (Track *t: s->tracks){
		for (TrackLayer *l: t->layers){
			if (!view->sel.has(l))
				continue;
			if (t->type == Track::Type::TIME)
				continue;

			paste_track_as_samples(ti, l, view);
			ti ++;
		}
	}
	s->action_manager->endActionGroup();
}

bool Clipboard::hasData()
{
	return (temp->tracks.num > 0);
}

bool Clipboard::canCopy(AudioView *view)
{
	return !view->sel.range.empty();// or (view->audio->GetNumSelectedSamples() > 0);
}

