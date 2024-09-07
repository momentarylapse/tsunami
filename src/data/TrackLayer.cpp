/*
 * Track.cpp
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#include "TrackLayer.h"
#include "Track.h"
#include "base.h"
#include "Song.h"
#include "CrossFade.h"
#include "Sample.h"
#include "SampleRef.h"
#include "TrackMarker.h"
#include "audio/AudioBuffer.h"
#include "../action/track/buffer/ActionTrackCreateBuffers.h"
#include "../action/track/buffer/ActionTrackEditBuffer.h"
#include "../action/track/layer/ActionTrackLayerMakeTrack.h"
#include "../action/track/layer/ActionTrackLayerMarkDominant.h"
#include "../action/track/midi/ActionTrackInsertMidi.h"
#include "../action/track/sample/ActionTrackAddSample.h"
#include "../action/track/sample/ActionTrackDeleteSample.h"
#include "../action/track/sample/ActionTrackEditSample.h"
#include "../action/track/midi/ActionTrackAddMidiNote.h"
#include "../action/track/midi/ActionTrackDeleteMidiNote.h"
#include "../action/track/midi/ActionTrackEditMidiNote.h"
#include "../action/track/marker/ActionTrackAddMarker.h"
#include "../action/track/marker/ActionTrackDeleteMarker.h"
#include "../action/track/marker/ActionTrackEditMarker.h"
#include "../lib/base/iter.h"
#include "../lib/base/algo.h"
#include "../lib/os/msg.h"

namespace tsunami {

TrackLayer::TrackLayer(Track *t) {
	//msg_write("  new TrackLayer " + p2s(this));
	track = t;
	type = t->type;
	channels = t->channels;
	muted = false;
}

TrackLayer::~TrackLayer() {
	//msg_write("  del TrackLayer " + p2s(this));
}

Range TrackLayer::range(int keep_notes) const {
	Range r = Range::NONE;

	for (AudioBuffer &b: buffers)
		r = r or b.range();

	if ((type == SignalType::Midi) and (midi.num > 0))
		r = r or midi.range(keep_notes);

	for (auto *m: weak(markers))
		r = r or m->range;

	for (auto *s: weak(samples))
		r = r or s->range();

	return r;
}

Song *TrackLayer::song() const {
	if (!track)
		return nullptr;
	return track->song;
}

int TrackLayer::version_number() const {
	if (!track)
		return 0;
	return base::find_index(weak(track->layers), const_cast<TrackLayer*>(this));
}

Array<Range> TrackLayer::active_version_ranges() const {
	Array<Range> r;
	bool active = true;
	int active_since = Range::BEGIN;
	for (auto &f: fades) {
		if (active and (f.mode == f.Outward)) {
			r.add(Range::to(active_since, f.range().end()));
		} else if (!active and (f.mode == f.Inward)) {
			active_since = f.position;
		}
		active = (f.mode == f.Inward);
	}
	if (active)
		r.add(Range::to(active_since, Range::END));
	return r;
}

Array<Range> TrackLayer::inactive_version_ranges() const {
	Array<Range> r;
	bool active = true;
	int active_since = Range::BEGIN;
	for (auto &f: fades) {
		if (active and (f.mode == f.Outward)) {
			active_since = f.range().end();
		} else if (!active and (f.mode == f.Inward)) {
			r.add(Range::to(active_since, f.range().start()));
		}
		active = (f.mode == f.Inward);
	}
	if (!active)
		r.add(Range::to(active_since, Range::END));
	return r;
}

Array<TrackMarker*> TrackLayer::markers_sorted() const {
	auto sorted = weak(markers);
	for (int i=0; i<sorted.num; i++)
		for (int j=i+1; j<sorted.num; j++)
			if (sorted[i]->range.offset > sorted[j]->range.offset)
				sorted.swap(i, j);
	return sorted;
}


void TrackLayer::read_buffers(AudioBuffer &buf, const Range &r, bool allow_ref) {
	buf.clear();
	buf.set_channels(channels);

	// is <r> inside a buffer?
	if (allow_ref) {
		for (AudioBuffer &b: buffers) {
			if (b.range().covers(r)) {
				int p0 = r.offset - b.offset;
				// set as reference to subarrays
				buf.set_as_ref(b, p0, r.length);
				return;
			}
		}
	}

	// create own...
	buf.resize(r.length);

	// fill with overlap
	for (AudioBuffer &b: buffers)
		buf.set(b, b.offset - r.offset, 1.0f);
}

void TrackLayer::read_buffers_fixed(AudioBuffer &buf, const Range &r) {
	if (r.length != buf.length)
		msg_error("TrackLayer.read_buffers_fixed: length mismatch");

	// fill with overlap
	for (AudioBuffer &b: buffers)
		buf.set(b, b.offset - r.offset, 1.0f);
}

// DEPRECATED
AudioBuffer TrackLayer::_read_buffers(const Range &r, bool allow_ref) {
	AudioBuffer buf;
	read_buffers(buf, r, allow_ref);
	return buf;
}


void TrackLayer::get_buffers(AudioBuffer &buf, const Range &r) {
	track->song->execute(new ActionTrackCreateBuffers(this, r));
	read_buffers(buf, r, true);
}

Action *TrackLayer::edit_buffers(AudioBuffer &buf, const Range &r) {
	get_buffers(buf, r);
	if (track->song->history_enabled())
		return new ActionTrackEditBuffer(this, r);
	return NULL;
}

void TrackLayer::edit_buffers_finish(Action *a) {
	if (a)
		track->song->execute(a);
}

shared<SampleRef> TrackLayer::add_sample_ref(int pos, Sample *sample) {
	return (SampleRef*)track->song->execute(new ActionTrackAddSample(this, pos, sample));
}

void TrackLayer::delete_sample_ref(SampleRef *ref) {
	track->song->execute(new ActionTrackDeleteSample(ref));
}

void TrackLayer::edit_sample_ref(SampleRef *ref, float volume, bool mute) {
	track->song->execute(new ActionTrackEditSample(ref, volume, mute));
}

// will take ownership of this instance!
void TrackLayer::add_midi_note(shared<MidiNote> n) {
	track->song->execute(new ActionTrackAddMidiNote(this, n));
}

void TrackLayer::add_midi_notes(const MidiNoteBuffer &notes) {
	track->song->begin_action_group("add midi notes");
	for (MidiNote *n: weak(notes))
		add_midi_note(n);
	track->song->end_action_group();
}

void TrackLayer::edit_midi_note(MidiNote *note, const Range &range, float pitch, float volume) {
	track->song->execute(new ActionTrackEditMidiNote(this, note, range, pitch, volume, note->stringno, note->flags));
}

void TrackLayer::midi_note_set_string(MidiNote *note, int stringno) {
	track->song->execute(new ActionTrackEditMidiNote(this, note, note->range, note->pitch, note->volume, stringno, note->flags));
}

void TrackLayer::midi_note_set_flags(MidiNote *note, int flags) {
	track->song->execute(new ActionTrackEditMidiNote(this, note, note->range, note->pitch, note->volume, note->stringno, flags));
}

void TrackLayer::delete_midi_note(const MidiNote *note) {
	for (auto&& [index,n]: enumerate(weak(midi)))
		if (n == note)
			track->song->execute(new ActionTrackDeleteMidiNote(this, index));
}

const shared<TrackMarker> TrackLayer::add_marker(const shared<TrackMarker> marker) {
	track->song->execute(new ActionTrackAddMarker(this, marker));
	return marker;
}

void TrackLayer::delete_marker(const TrackMarker *marker) {
	for (auto&& [index,m]: enumerate(weak(markers)))
		if (m == marker)
			track->song->execute(new ActionTrackDeleteMarker(this, index));
}

void TrackLayer::edit_marker(const TrackMarker *marker, const Range &range, const string &text) {
	track->song->execute(new ActionTrackEditMarker(this, (TrackMarker*)marker, range, text));
}

void TrackLayer::make_own_track() {
	track->song->execute(new ActionTrackLayerMakeTrack(this));
}

void TrackLayer::version_activate(const Range &range, bool activate) {
	track->song->execute(new ActionTrackLayerActivateVersion(this, range, activate));
}

void TrackLayer::set_muted(bool m) {
	// todo
	muted = m;
	out_changed.notify();
	track->song->out_changed.notify();
}

bool TrackLayer::is_main() const {
	return (this == track->layers[0]);
}

void TrackLayer::insert_midi_data(int offset, const MidiNoteBuffer& midi) {
	track->song->execute(new ActionTrackInsertMidi(this, offset, midi));
}


base::set<const TrackLayer*> layer_set(const Array<TrackLayer*> &layers) {
	base::set<const TrackLayer*> lset;
	for (auto l: layers)
		lset.add(l);
	return lset;
}

}
