/*
 * Clipboard.cpp
 *
 *  Created on: 21.12.2012
 *      Author: michi
 */

#include "Clipboard.h"
#include "../view/audioview/AudioView.h"
#include "../action/track/sample/ActionTrackPasteAsSample.h"
#include "../Session.h"
#include "../data/base.h"
#include "../data/Song.h"
#include "../data/Track.h"
#include "../data/TrackLayer.h"
#include "../data/TrackMarker.h"
#include "../data/Sample.h"
#include "../data/SampleRef.h"
#include "../data/SongSelection.h"
#include "../data/rhythm/Bar.h"
#include "../data/rhythm/Beat.h"
#include "../lib/base/iter.h"
#include "../lib/hui/language.h"
#include <assert.h>

namespace tsunami {

Clipboard::Clipboard() {
	temp = new Song(Session::GLOBAL, DEFAULT_SAMPLE_RATE);
}

void Clipboard::clear() {
	if (temp->tracks.num > 0) {
		temp->reset();
		out_changed.notify();
	}
	ref_uid.clear();
}

void Clipboard::_copy_append_track(TrackLayer *source, AudioView *view, int offset) {
	if (source->type == SignalType::Beats)
		return;

	TrackLayer *target = temp->add_track(source->type)->layers[0].get();

	if (source->type == SignalType::Audio) {
		AudioBuffer buf;
		target->buffers.add(buf);
		source->read_buffers(target->buffers[0], view->sel.range(), false);
	} else if (source->type == SignalType::Midi) {
		target->midi = source->midi.get_notes_by_selection(view->sel).duplicate(-offset);
		target->midi.samples = view->sel.range().length;
		target->midi.sanify(view->sel.range());
	}
	for (auto m: weak(source->markers))
		if (view->sel.has(m))
			target->markers.add(m->copy(-offset));

	ref_uid.add(-1);
}

static Range find_selection_range(AudioView *view) {
	if (!view->sel.range().is_empty())
		return view->sel.range();

	// no range selected -> selected items
	int start = view->song->range().end();
	int end = view->song->range().start();
	for (auto *t: weak(view->song->tracks))
		for (auto *l: weak(t->layers))
			if (view->sel.has(l)) {
				for (MidiNote *n: weak(l->midi))
					if (view->sel.has(n)) {
						start = min(start, n->range.offset);
						end = min(end, n->range.end());
					}
				for (auto m: weak(l->markers))
					if (view->sel.has(m)) {
						start = min(start, m->range.offset);
						end = min(end, m->range.end());
					}
			}
	return Range::to(start, end);
}

static void copy_bars(AudioView *view, Song *d, const Range &r) {
	auto s = view->song;
	for (Bar *b: weak(s->bars))
		//if (view->sel.has(b)) {
		if (r.covers(b->range())) {
			// fill gap before bar with pause
			if (d->bars.range().end() < b->range().start() - r.start())
				d->bars.add(new Bar(b->range().start() - r.start() - d->bars.range().end(), 0, 0));
			// the actual bar
			d->bars.add(b->copy());
		} else if (r.overlaps(b->range())) {
			auto rr = r.intersect(b->range());
			// TODO split bar more accurately!
			int beats = (int)((double)b->beats.num * (double)rr.length / (double)b->length + 0.4);
			d->bars.add(new Bar(rr.length, beats, b->divisor));
		}
	// fill with pause at end
	if (d->bars.range().end() < r.length)
		d->bars.add(new Bar(r.length - d->bars.range().end(), 0, 0));
	d->bars._update_offsets();
}

void Clipboard::copy(AudioView *view) {
	if (!can_copy(view))
		return;
	clear();

	Song *s = view->song;

	temp->sample_rate = s->sample_rate;

	Range sel_range = find_selection_range(view);
	copy_bars(view, temp.get(), sel_range);

	for (Track *t: weak(s->tracks))
		for (TrackLayer *l: weak(t->layers))
			if (view->sel.has(l))
				_copy_append_track(l, view, sel_range.offset);

	out_changed.notify();
}

void Clipboard::paste_track(TrackLayer *source, TrackLayer *target, int offset) {
	offset -= temp_offset;
	if (target->type == SignalType::Audio) {
		Range r = Range(offset, source->buffers[0].length);
		AudioBuffer buf;
		auto *a = target->edit_buffers(buf, r);
		buf.set(source->buffers[0], 0, 1.0f);
		target->edit_buffers_finish(a);
	} else if (target->type == SignalType::Midi) {
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
		if (target->type == SignalType::Audio) {
			MidiNoteBuffer midi;
			Sample *sample = (Sample*)s->execute(new ActionTrackPasteAsSample(target, offset, source->buffers[0], true));
			ref_uid[source_index] = sample->uid;
		} else if (target->type == SignalType::Midi) {
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
			if (l->type == SignalType::Beats)
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
	for (auto&& [i,ls]: enumerate(source_list))
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
	for (auto&& [i,ls]: enumerate(source_list))
		paste_track_as_samples(ls, i, target_list[i], offset);

	s->end_action_group();
}

void Clipboard::paste_insert_time(AudioView *view) {
	if (!has_data())
		return;
	Song *s = view->song;
	s->begin_action_group("paste insert time");

	int offset = view->cursor_pos();
	int index = 0;
	foreachi (Bar *b, weak(s->bars), i)
		if (b->offset >= offset) {
			index = i;
			break;
		}

	//if (temp->bars.num )
	for (Bar *b: weak(temp->bars))
		s->add_bar(index ++, *b, BarEditMode::InsertSilence);

	paste(view);

	s->end_action_group();
}

void Clipboard::paste_aligned_to_beats(AudioView *view) {
	if (!has_data())
		return;
	Song *s = view->song;

	Array<TrackLayer*> source_list, target_list;
	if (!prepare_layer_map(view, source_list, target_list))
		return;
	temp->bars._update_offsets();


	s->begin_action_group("paste aligned to beats");

	int offset = view->sel.range().start();

	auto source_beats = temp->bars.get_beats(Range::ALL);
	auto dest_beats = view->song->bars.get_beats(Range::ALL);

	int dest_first_beat = -1;
	int dest_offset = offset;
	for (auto&& [i,b]: enumerate(dest_beats))
		if (b.range.start() <= offset) {
			dest_first_beat = i;
			dest_offset = offset - b.range.start();
		}

	auto remap = [&] (int x) {
		// nothing to match
		if (source_beats.num == 0 or dest_first_beat < 0)
			return x + offset;
		// squeeze to fit before first beat-match
		if (x <= 0)
			return x + offset;
		if (x < source_beats[0].range.start())
			return offset + (int)( (double)x * (double)dest_offset / (double) source_beats[0].range.start() ); // FIXME

		for (auto&& [i,bs]: enumerate(source_beats))
			if (bs.range.is_inside(x) and (dest_first_beat + i > 0) and (dest_first_beat + i < dest_beats.num)) {
				auto &bd = dest_beats[dest_first_beat + i];
				return bd.range.offset + (int)( (double)(x - bs.range.offset) * (double)(bd.range.length) / (double)(bs.range.length) );
			}
		int overlap_beats = min(source_beats.num, dest_beats.num - dest_first_beat);
		return (x - source_beats[overlap_beats - 1].range.end()) + dest_beats[dest_first_beat + overlap_beats - 1].range.end();
	};

	auto paste_layer = [remap] (TrackLayer *source, TrackLayer *dest) {
		if (dest->type != SignalType::Midi)
			return;

		for (auto n: source->midi) {
			auto c = n->copy();
			c->range = Range::to(remap(n->range.offset), remap(n->range.end()));
			dest->add_midi_note(c);
		}
	};

	for (auto&& [i,source]: enumerate(source_list))
		paste_layer(source, target_list[i]);

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

}

