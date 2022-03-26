/*
 * Clipboard.cpp
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#include "Clipboard.h"
#include "../View/AudioView/AudioView.h"
#include "../Action/Track/Sample/ActionTrackPasteAsSample.h"
#include "../Session.h"
#include "../Data/base.h"
#include "../Data/Song.h"
#include "../Data/Track.h"
#include "../Data/TrackLayer.h"
#include "../Data/TrackMarker.h"
#include "../Data/Sample.h"
#include "../Data/SongSelection.h"
#include "../Data/Rhythm/Bar.h"
#include <assert.h>

Clipboard::Clipboard() {
	temp = new Song(Session::GLOBAL, DEFAULT_SAMPLE_RATE);
}

void Clipboard::clear() {
	if (temp->tracks.num > 0) {
		temp->reset();
		notify();
	}
	ref_uid.clear();
}

void Clipboard::append_track(TrackLayer *source, AudioView *view, int offset) {
	if (source->type == SignalType::BEATS)
		return;

	TrackLayer *target = temp->add_track(source->type)->layers[0].get();

	if (source->type == SignalType::AUDIO) {
		AudioBuffer buf;
		target->buffers.add(buf);
		source->read_buffers(target->buffers[0], view->sel.range(), false);
	} else if (source->type == SignalType::MIDI) {
		target->midi = source->midi.get_notes_by_selection(view->sel).duplicate(-offset);
		target->midi.samples = view->sel.range().length;
		target->midi.sanify(view->sel.range());
	}
	for (auto m: weak(source->markers))
		if (view->sel.has(m))
			target->markers.add(m->copy(-offset));

	ref_uid.add(-1);
}

static int find_offset(AudioView *view) {
	if (!view->sel.range().is_empty())
		return view->sel.range().offset;

	// offset to first selected item
	int offset = view->song->range().end();
	for (auto *t: weak(view->song->tracks))
		for (auto *l: weak(t->layers))
			if (view->sel.has(l)) {
				for (MidiNote *n: weak(l->midi))
					if (view->sel.has(n))
						offset = min(offset, n->range.offset);
				for (auto m: weak(l->markers))
					if (view->sel.has(m))
						offset = min(offset, m->range.offset);
			}

	return offset;
}


void Clipboard::copy(AudioView *view) {
	if (!can_copy(view))
		return;
	clear();

	Song *s = view->song;

	temp->sample_rate = s->sample_rate;

	int offset = find_offset(view);

	for (Bar *b: weak(s->bars))
		if (view->sel.has(b)) {
			if (temp->bars.num == 0 and b->offset > offset)
				temp->bars.add(new Bar(b->offset - offset, 0, 0));
			temp->bars.add(b->copy());
		}

	for (Track *t: weak(s->tracks))
		for (TrackLayer *l: weak(t->layers))
			if (view->sel.has(l))
				append_track(l, view, offset);

	notify();
}

void Clipboard::paste_track(TrackLayer *source, TrackLayer *target, int offset) {
	if (target->type == SignalType::AUDIO) {
		Range r = Range(offset, source->buffers[0].length);
		AudioBuffer buf;
		auto *a = target->edit_buffers(buf, r);
		buf.set(source->buffers[0], 0, 1.0f);
		target->edit_buffers_finish(a);
	} else if (target->type == SignalType::MIDI) {
		for (MidiNote *n: weak(source->midi))
			target->add_midi_note(n->copy(offset));
	}
	for (auto m: weak(source->markers))
		target->add_marker(m->copy(offset));
}

void Clipboard::paste_track_as_samples(TrackLayer *source, int source_index, TrackLayer *target, int offset) {
	Song *s = target->track->song;
	//Track *source = temp->tracks[source_index];

	Sample* ref = s->get_sample_by_uid(ref_uid[source_index]);
	if (ref) {
		target->add_sample_ref(offset, ref);
	} else {
		if (target->type == SignalType::AUDIO) {
			MidiNoteBuffer midi;
			Sample *sample = (Sample*)s->execute(new ActionTrackPasteAsSample(target, offset, source->buffers[0], true));
			ref_uid[source_index] = sample->uid;
		} else if (target->type == SignalType::MIDI) {
			Sample *sample = (Sample*)s->execute(new ActionTrackPasteAsSample(target, offset, source->midi, true));
			ref_uid[source_index] = sample->uid;
		}
	}
}

bool Clipboard::prepare_layer_map(AudioView *view, Array<TrackLayer*> &source_list, Array<TrackLayer*> &target_list) {
	Array<string> temp_type, dest_type;
	for (Track *t: weak(view->song->tracks)) {
		for (TrackLayer *l: weak(t->layers)) {
			if (!view->sel.has(l))
				continue;
			if (l->type == SignalType::BEATS)
				continue;
			target_list.add(l);
			dest_type.add(signal_type_name(l->type));
		}
	}

	for (Track *t: weak(temp->tracks)) {
		source_list.add(t->layers[0].get());
		temp_type.add(signal_type_name(t->type));
	}

	if (dest_type.num != temp->tracks.num) {
		view->session->e(format(_("%d tracks selected for pasting (ignoring the metronome), but %d tracks in clipboard"), dest_type.num, temp->tracks.num));
		return false;
	}
	string t1 = "[" + implode(temp_type, ", ") + "]";
	string t2 = "[" + implode(dest_type, ", ") + "]";
	if (t1 != t2){
		view->session->e(format(_("Track types in clipboard (%s) don't match those you want to paste into (%s)"), t1, t2));
		return false;
	}
	return true;
}

void Clipboard::paste(AudioView *view) {
	if (!has_data())
		return;
	Song *s = view->song;

	Array<TrackLayer*> source_list, target_list;
	if (!prepare_layer_map(view, source_list, target_list))
		return;


	s->begin_action_group("paste");

	int offset = view->sel.range().start();
	foreachi(auto *ls, source_list, i)
		paste_track(ls, target_list[i], offset);

	s->end_action_group();
}

void Clipboard::paste_as_samples(AudioView *view) {
	if (!has_data())
		return;
	Song *s = view->song;

	Array<TrackLayer*> source_list, target_list;
	if (!prepare_layer_map(view, source_list, target_list))
		return;


	s->begin_action_group("paste as samples");

	int offset = view->sel.range().start();
	foreachi(auto *ls, source_list, i)
		paste_track_as_samples(ls, i, target_list[i], offset);

	s->end_action_group();
}

void Clipboard::paste_with_time(AudioView *view) {
	if (!has_data())
		return;
	Song *s = view->song;
	s->begin_action_group("paste with time");

	int offset = view->cursor_pos();
	int index = 0;
	foreachi (Bar *b, weak(s->bars), i)
		if (b->offset >= offset) {
			index = i;
			break;
		}

	//if (temp->bars.num )
	for (Bar *b: weak(temp->bars))
		s->add_bar(index ++, *b, Bar::EditMode::INSERT_SILENCE);

	paste(view);

	s->end_action_group();
}

bool Clipboard::has_data() {
	return (temp->tracks.num > 0) or (temp->bars.num > 0);
}

bool Clipboard::can_copy(AudioView *view) {
	if (!view->sel.range().is_empty())
		return true;
	if (view->sel._notes.num > 0)
		return true;
	if (view->sel._markers.num > 0)
		return true;
	//if (view->sel._samples.num > 0)
	//	return true;
	return false;
}

