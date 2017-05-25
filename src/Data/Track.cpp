/*
 * Track.cpp
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#include "Track.h"
#include "../Plugins/Effect.h"
#include "../Audio/Synth/Synthesizer.h"
#include "../Action/Track/Buffer/ActionTrackCreateBuffers.h"
#include "../lib/hui/hui.h"
#include "../lib/threads/Mutex.h"
#include "../Action/Track/Data/ActionTrackEditName.h"
#include "../Action/Track/Data/ActionTrackEditMuted.h"
#include "../Action/Track/Data/ActionTrackEditVolume.h"
#include "../Action/Track/Data/ActionTrackEditPanning.h"
#include "../Action/Track/Data/ActionTrackSetInstrument.h"
#include "../Action/Track/Midi/ActionTrackInsertMidi.h"
#include "../Action/Track/Midi/ActionTrackAddMidiEffect.h"
#include "../Action/Track/Midi/ActionTrackDeleteMidiEffect.h"
#include "../Action/Track/Midi/ActionTrackEditMidiEffect.h"
#include "../Action/Track/Midi/ActionTrackToggleMidiEffectEnabled.h"
#include "../Action/Track/Sample/ActionTrackAddSample.h"
#include "../Action/Track/Sample/ActionTrackDeleteSample.h"
#include "../Action/Track/Sample/ActionTrackEditSample.h"
#include "../Action/Track/Synthesizer/ActionTrackSetSynthesizer.h"
#include "../Action/Track/Synthesizer/ActionTrackEditSynthesizer.h"
#include "../Action/Track/Synthesizer/ActionTrackDetuneSynthesizer.h"
#include "../Action/Track/Effect/ActionTrackAddEffect.h"
#include "../Action/Track/Effect/ActionTrackDeleteEffect.h"
#include "../Action/Track/Effect/ActionTrackEditEffect.h"
#include "../Action/Track/Effect/ActionTrackToggleEffectEnabled.h"
#include "../Action/Track/Marker/ActionTrackAddMarker.h"
#include "../Action/Track/Marker/ActionTrackDeleteMarker.h"
#include "../Action/Track/Marker/ActionTrackMoveMarker.h"
#include "../Action/Track/Midi/ActionTrackAddMidiNote.h"
#include "../Action/Track/Midi/ActionTrackDeleteMidiNote.h"
#include "../Tsunami.h"
#include "../Plugins/PluginManager.h"


string track_type(int type)
{
	if (type == Track::TYPE_AUDIO)
		return _("Audio");
	if (type == Track::TYPE_MIDI)
		return _("Midi");
	if (type == Track::TYPE_TIME)
		return _("Metronome");
	return "???";
}

const string Track::MESSAGE_ADD_EFFECT = "AddEffect";
const string Track::MESSAGE_DELETE_EFFECT = "DeleteEffect";
const string Track::MESSAGE_ADD_MIDI_EFFECT = "AddMidiEffect";
const string Track::MESSAGE_DELETE_MIDI_EFFECT = "DeleteMidiEffect";

Track::Track(int _type, Synthesizer *_synth) :
	Observable("Track")
{
	type = _type;
	muted = false;
	volume = 1;
	panning = 0;
	song = NULL;

	volume = 1;
	muted = false;

	synth = _synth;
	if (synth)
		synth->song = song;
}

//tsunami->plugin_manager->CreateSynthesizer("Dummy", song)


Track::~Track()
{
	midi.deep_clear();

	for (Effect *f: fx)
		delete(f);
	fx.clear();

	for (SampleRef *r: samples)
		delete(r);
	samples.clear();

	if (synth)
		delete(synth);
}

Range Track::getRange()
{
	Range r = Range::EMPTY;

	for (TrackLayer &l: layers)
		for (BufferBox &b: l.buffers)
			r = r or b.range();

	for (SampleRef *s: samples)
		r = r or s->range();

	if ((type == TYPE_MIDI) and (midi.num > 0))
		r = r or midi.getRange(synth->keep_notes);

	return r;
}

string Track::getNiceName()
{
	if (name.num > 0)
		return name;
	if (type == TYPE_TIME)
		return _("Metronome");
	int n = get_track_index(this);
	return format(_("Track %d"), n+1);
}

int Track::get_index()
{
	if (song){
		foreachi(Track *t, song->tracks, i)
			if (this == t)
				return i;
	}
	return -1;
}

BufferBox Track::readBuffers(int layer_no, const Range &r)
{
	BufferBox buf;

	// is <r> inside a buffer?
	for (BufferBox &b: layers[layer_no].buffers){
		int p0 = r.offset - b.offset;
		int p1 = r.offset - b.offset + r.length;
		if ((p0 >= 0) and (p1 <= b.length)){
			// set as reference to subarrays
			buf.set_as_ref(b, p0, p1 - p0);
			return buf;
		}
	}

	// create own...
	buf.resize(r.length);

	// fill with overlap
	for (BufferBox &b: layers[layer_no].buffers)
		buf.set(b, b.offset - r.offset, 1.0f);

	return buf;
}

BufferBox Track::readBuffersCol(const Range &r)
{
	BufferBox buf;

	// is <r> inside a single buffer?
	int num_inside = 0;
	int inside_layer, inside_no;
	int inside_p0, inside_p1;
	bool intersected = false;
	foreachi(TrackLayer &l, layers, li)
		foreachi(BufferBox &b, l.buffers, bi){
			if (b.range().covers(r)){
				num_inside ++;
				inside_layer = li;
				inside_no = bi;
				inside_p0 = r.offset - b.offset;
				inside_p1 = r.offset - b.offset + r.length;
			}else if (b.range().overlaps(r))
				intersected = true;
		}
	if ((num_inside == 1) and (!intersected)){
		// set as reference to subarrays
		buf.set_as_ref(layers[inside_layer].buffers[inside_no], inside_p0, inside_p1 - inside_p0);
		return buf;
	}

	// create own...
	buf.resize(r.length);

	// fill with overlap
	for (TrackLayer &l: layers)
		for (BufferBox &b: l.buffers)
			buf.add(b, b.offset - r.offset, 1.0f, 0.0f);

	return buf;
}

BufferBox Track::getBuffers(int layer_no, const Range &r)
{
	song->execute(new ActionTrackCreateBuffers(this, layer_no, r));
	return readBuffers(layer_no, r);
}

void Track::updatePeaks()
{
	for (TrackLayer &l: layers)
		for (BufferBox &b: l.buffers)
			b.update_peaks();
}

void Track::invalidateAllPeaks()
{
	for (TrackLayer &l: layers)
		for (BufferBox &b: l.buffers)
			b.peaks.clear();
}

SampleRef *Track::addSampleRef(int pos, Sample* sample)
{
	return (SampleRef*)song->execute(new ActionTrackAddSample(this, pos, sample));
}

void Track::deleteSampleRef(int index)
{
	song->execute(new ActionTrackDeleteSample(this, index));
}

void Track::editSampleRef(int index, float volume, bool mute)
{
	song->execute(new ActionTrackEditSample(this, index, volume, mute));
}

void Track::addMidiNote(const MidiNote &n)
{
	song->execute(new ActionTrackAddMidiNote(this, n));
}

void Track::addMidiNotes(const MidiData &notes)
{
	song->action_manager->beginActionGroup();
	for (MidiNote *n: notes)
		addMidiNote(*n);
	song->action_manager->endActionGroup();
}

void Track::deleteMidiNote(int index)
{
	song->execute(new ActionTrackDeleteMidiNote(this, index));
}

void Track::setName(const string& name)
{
	song->execute(new ActionTrackEditName(this, name));
}

void Track::setInstrument(const Instrument& instrument)
{
	song->execute(new ActionTrackSetInstrument(this, instrument));
}

void Track::setMuted(bool muted)
{
	song->execute(new ActionTrackEditMuted(this, muted));
}

void Track::setVolume(float volume)
{
	song->execute(new ActionTrackEditVolume(this, volume));
}

void Track::setPanning(float panning)
{
	song->execute(new ActionTrackEditPanning(this, panning));
}

void Track::insertMidiData(int offset, const MidiData& midi)
{
	song->execute(new ActionTrackInsertMidi(this, offset, midi));
}

void Track::addEffect(Effect *effect)
{
	song->execute(new ActionTrackAddEffect(this, effect));
}

// execute after editing...
void Track::editEffect(int index, const string &param_old)
{
	song->execute(new ActionTrackEditEffect(this, index, param_old, fx[index]));
}

void Track::enableEffect(int index, bool enabled)
{
	if (fx[index]->enabled != enabled)
		song->execute(new ActionTrackToggleEffectEnabled(this, index));
}

void Track::deleteEffect(int index)
{
	song->execute(new ActionTrackDeleteEffect(this, index));
}

void Track::addMidiEffect(MidiEffect *effect)
{
	song->execute(new ActionTrackAddMidiEffect(this, effect));
}

// execute after editing...
void Track::editMidiEffect(int index, const string &param_old)
{
	song->execute(new ActionTrackEditMidiEffect(this, index, param_old, midi.fx[index]));
}

void Track::enableMidiEffect(int index, bool enabled)
{
	if (midi.fx[index]->enabled != enabled)
		song->execute(new ActionTrackToggleMidiEffectEnabled(this, index));
}

void Track::deleteMidiEffect(int index)
{
	song->execute(new ActionTrackDeleteMidiEffect(this, index));
}

void Track::setSynthesizer(Synthesizer *_synth)
{
	song->execute(new ActionTrackSetSynthesizer(this, _synth));
}

// execute after editing...
void Track::editSynthesizer(const string &param_old)
{
	song->execute(new ActionTrackEditSynthesizer(this, param_old));
}

void Track::detuneSynthesizer(int pitch, float dpitch, bool all_octaves)
{
	song->execute(new ActionTrackDetuneSynthesizer(this, pitch, dpitch, all_octaves));
}

void Track::addMarker(int pos, const string &text)
{
	song->execute(new ActionTrackAddMarker(this, pos, text));
}

void Track::deleteMarker(int index)
{
	song->execute(new ActionTrackDeleteMarker(this, index));
}

void Track::moveMarker(int index, int pos)
{
	song->execute(new ActionTrackMoveMarker(this, index, pos));
}



