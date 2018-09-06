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
#include "SampleRef.h"
#include "Audio/AudioBuffer.h"
#include "../Action/Track/Buffer/ActionTrackCreateBuffers.h"
#include "../Action/Track/Layer/ActionTrackLayerMakeTrack.h"
#include "../Action/Track/Layer/ActionTrackLayerMarkDominant.h"
#include "../Action/Track/Midi/ActionTrackInsertMidi.h"
#include "../Action/Track/Sample/ActionTrackAddSample.h"
#include "../Action/Track/Sample/ActionTrackDeleteSample.h"
#include "../Action/Track/Sample/ActionTrackEditSample.h"
#include "../Action/Track/Midi/ActionTrackAddMidiNote.h"
#include "../Action/Track/Midi/ActionTrackDeleteMidiNote.h"
//#include "../Plugins/PluginManager.h"
//#include "../Tsunami.h"
#include "../lib/hui/hui.h"
#include "../lib/threads/Mutex.h"


TrackLayer::TrackLayer(){}

TrackLayer::TrackLayer(Track *t)
{
	track = t;
	type = t->type;
	channels = t->channels;
}

TrackLayer::~TrackLayer()
{
	midi.deep_clear();

	for (SampleRef *r: samples)
		delete(r);
	samples.clear();
}

Range TrackLayer::range(int keep_notes) const
{
	Range r = Range::EMPTY;

	for (AudioBuffer &b: buffers)
		r = r or b.range();

	if ((type == SignalType::MIDI) and (midi.num > 0))
		r = r or midi.range(keep_notes);

	for (SampleRef *s: samples)
		r = r or s->range();

	return r;
}

Song *TrackLayer::song() const
{
	if (!track)
		return nullptr;
	return track->song;
}

int TrackLayer::version_number() const
{
	if (!track)
		return 0;
	foreachi (TrackLayer *l, track->layers, i)
		if (l == this)
			return i;
	return 0;
}


void TrackLayer::readBuffers(AudioBuffer &buf, const Range &r, bool allow_ref)
{
	buf.clear_x(channels);

	// is <r> inside a buffer?
	if (allow_ref){
		for (AudioBuffer &b: buffers){
			if (b.range().covers(r)){
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

void TrackLayer::read_buffers_fixed(AudioBuffer &buf, const Range &r)
{
	if (r.length != buf.length)
		msg_error("TrackLayer.read_buffers_fixed: length mismatch");

	// fill with overlap
	for (AudioBuffer &b: buffers)
		buf.set(b, b.offset - r.offset, 1.0f);
}

// DEPRECATED
AudioBuffer TrackLayer::_readBuffers(const Range &r, bool allow_ref)
{
	AudioBuffer buf;
	readBuffers(buf, r, allow_ref);
	return buf;
}


void TrackLayer::getBuffers(AudioBuffer &buf, const Range &r)
{
	track->song->execute(new ActionTrackCreateBuffers(this, r));
	readBuffers(buf, r, true);
}

// DEPRECATED
AudioBuffer TrackLayer::_getBuffers(const Range &r)
{
	AudioBuffer b;
	getBuffers(b, r);
	return b;
}

SampleRef *TrackLayer::addSampleRef(int pos, Sample* sample)
{
	return (SampleRef*)track->song->execute(new ActionTrackAddSample(this, pos, sample));
}

void TrackLayer::deleteSampleRef(SampleRef *ref)
{
	track->song->execute(new ActionTrackDeleteSample(ref));
}

void TrackLayer::editSampleRef(SampleRef *ref, float volume, bool mute)
{
	track->song->execute(new ActionTrackEditSample(ref, volume, mute));
}

// will take ownership of this instance!
void TrackLayer::addMidiNote(MidiNote *n)
{
	track->song->execute(new ActionTrackAddMidiNote(this, n));
}

void TrackLayer::addMidiNotes(const MidiNoteBuffer &notes)
{
	track->song->beginActionGroup();
	for (MidiNote *n: notes)
		addMidiNote(n);
	track->song->endActionGroup();
}

void TrackLayer::deleteMidiNote(const MidiNote *note)
{
	foreachi(MidiNote *n, midi, index)
		if (n == note)
			track->song->execute(new ActionTrackDeleteMidiNote(this, index));
}

void TrackLayer::make_own_track()
{
	track->song->execute(new ActionTrackLayerMakeTrack(this));
}

void TrackLayer::mark_dominant(const Range &range)
{
	track->song->execute(new ActionTrackLayerMarkDominant(this, range));
}

bool TrackLayer::is_main()
{
	return (this == track->layers[0]);
}

void TrackLayer::insertMidiData(int offset, const MidiNoteBuffer& midi)
{
	track->song->execute(new ActionTrackInsertMidi(this, offset, midi));
}
