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
#include "../Action/Track/Data/ActionTrackEditName.h"
#include "../Action/Track/Data/ActionTrackEditMuted.h"
#include "../Action/Track/Data/ActionTrackEditVolume.h"
#include "../Action/Track/Data/ActionTrackEditPanning.h"
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
#include "../Action/Track/Effect/ActionTrackAddEffect.h"
#include "../Action/Track/Effect/ActionTrackDeleteEffect.h"
#include "../Action/Track/Effect/ActionTrackEditEffect.h"
#include "../Action/Track/Effect/ActionTrackToggleEffectEnabled.h"
#include "../Action/Track/Marker/ActionTrackAddMarker.h"
#include "../Action/Track/Marker/ActionTrackDeleteMarker.h"
#include "../Action/Track/Marker/ActionTrackMoveMarker.h"
#include "../Action/Track/Bar/ActionTrackAddBar.h"
#include "../Action/Track/Bar/ActionTrackEditBar.h"
#include "../Action/Track/Bar/ActionTrackDeleteBar.h"
#include "../Action/Track/Midi/ActionTrackAddMidiEvent.h"
#include "../Action/Track/Midi/ActionTrackDeleteMidiEvent.h"


string track_type(int type)
{
	if (type == Track::TYPE_AUDIO)
		return _("Audio");
	if (type == Track::TYPE_MIDI)
		return _("Midi");
	if (type == Track::TYPE_TIME)
		return _("Metronom");
	return "???";
}

const string Track::MESSAGE_ADD_EFFECT = "AddEffect";
const string Track::MESSAGE_DELETE_EFFECT = "DeleteEffect";
const string Track::MESSAGE_ADD_MIDI_EFFECT = "AddMidiEffect";
const string Track::MESSAGE_DELETE_MIDI_EFFECT = "DeleteMidiEffect";

Track::Track() :
	Observable("Track")
{
	type = TYPE_AUDIO;
	muted = false;
	volume = 1;
	panning = 0;
	root = NULL;
	is_selected = false;

	volume = 1;
	muted = false;

	synth = CreateSynthesizer("Dummy");
}



// destructor...
void Track::reset()
{
	msg_db_f("Track.Reset",1);
	levels.clear();
	name.clear();
	volume = 1;
	muted = false;
	panning = 0;
	bars.clear();
	foreach(Effect *f, fx)
		delete(f);
	fx.clear();
	samples.clear();
	if (synth)
		delete(synth);
	synth = CreateSynthesizer("Dummy");
}

Track::~Track()
{
	reset();
	if (synth)
		delete(synth);
}

Range Track::getRangeUnsafe()
{
	int min =  1073741824;
	int max = -1073741824;
	foreach(TrackLevel &l, levels)
		if (l.buffers.num > 0){
			min = min(l.buffers[0].offset, min);
			max = max(l.buffers.back().range().end(), max);
		}
	foreach(SampleRef *s, samples){
		if (s->pos < min)
			min = s->pos;
		int smax = s->pos + s->buf->num + s->rep_num * s->rep_delay;
		if (smax > max)
			max = smax;
	}
	Range r = Range(min, max - min);

	if ((type == TYPE_TIME) and (bars.num > 0))
		r = r or bars.getRange();

	if ((type == TYPE_MIDI) and (midi.num > 0))
		r = r or midi.getRange(synth->keep_notes);

	return r;
}

Range Track::getRange()
{
	Range r = getRangeUnsafe();
	if (r.length() < 0)
		return Range::EMPTY;
	return r;
}

string Track::getNiceName()
{
	if (name.num > 0)
		return name;
	if (type == TYPE_TIME)
		return _("Metronom");
	int n = get_track_index(this);
	return format(_("Spur %d"), n+1);
}

int Track::get_index()
{
	if (root){
		foreachi(Track *t, root->tracks, i)
			if (this == t)
				return i;
	}
	return -1;
}

BufferBox Track::readBuffers(int level_no, const Range &r)
{
	BufferBox buf;
	msg_db_f("Track.ReadBuffers", 1);

	// is <r> inside a buffer?
	foreach(BufferBox &b, levels[level_no].buffers){
		int p0 = r.offset - b.offset;
		int p1 = r.offset - b.offset + r.num;
		if ((p0 >= 0) and (p1 <= b.num)){
			// set as reference to subarrays
			buf.set_as_ref(b, p0, p1 - p0);
			return buf;
		}
	}

	// create own...
	buf.resize(r.num);

	// fill with overlapp
	foreach(BufferBox &b, levels[level_no].buffers)
		buf.set(b, b.offset - r.offset, 1.0f);

	return buf;
}

BufferBox Track::readBuffersCol(const Range &r)
{
	BufferBox buf;
	msg_db_f("Track.ReadBuffersCol", 1);

	// is <r> inside a single buffer?
	int num_inside = 0;
	int inside_level, inside_no;
	int inside_p0, inside_p1;
	bool intersected = false;
	foreachi(TrackLevel &l, levels, li)
		foreachi(BufferBox &b, l.buffers, bi){
			if (b.range().covers(r)){
				num_inside ++;
				inside_level = li;
				inside_no = bi;
				inside_p0 = r.offset - b.offset;
				inside_p1 = r.offset - b.offset + r.num;
			}else if (b.range().overlaps(r))
				intersected = true;
		}
	if ((num_inside == 1) and (!intersected)){
		// set as reference to subarrays
		buf.set_as_ref(levels[inside_level].buffers[inside_no], inside_p0, inside_p1 - inside_p0);
		return buf;
	}

	// create own...
	buf.resize(r.num);

	// fill with overlapp
	foreach(TrackLevel &l, levels)
		foreach(BufferBox &b, l.buffers)
			buf.add(b, b.offset - r.offset, 1.0f, 0.0f);

	return buf;
}

BufferBox Track::getBuffers(int level_no, const Range &r)
{
	root->execute(new ActionTrackCreateBuffers(this, level_no, r));
	return readBuffers(level_no, r);
}

void Track::updatePeaks(int mode)
{
	foreach(TrackLevel &l, levels)
		foreach(BufferBox &b, l.buffers)
			b.update_peaks(mode);
}

void Track::invalidateAllPeaks()
{
	foreach(TrackLevel &l, levels)
		foreach(BufferBox &b, l.buffers)
			b.invalidate_peaks(b.range());
}

SampleRef *Track::addSample(int pos, int index)
{
	return (SampleRef*)root->execute(new ActionTrackAddSample(this, pos, index));
}

void Track::deleteSample(int index)
{
	root->execute(new ActionTrackDeleteSample(this, index));
}

void Track::editSample(int index, float volume, bool mute, int rep_num, int rep_delay)
{
	root->execute(new ActionTrackEditSample(this, index, volume, mute, rep_num, rep_delay));
}

void Track::addMidiNote(const MidiNote &n)
{
	root->action_manager->beginActionGroup();
	addMidiEvent(MidiEvent(n.range.offset, n.pitch, n.volume));
	addMidiEvent(MidiEvent(n.range.end(), n.pitch, 0));
	root->action_manager->endActionGroup();
}

void Track::addMidiEvent(const MidiEvent &e)
{
	root->execute(new ActionTrackAddMidiEvent(this, e));
}

void Track::addMidiEvents(const MidiData &events)
{
	root->action_manager->beginActionGroup();
	foreach(MidiEvent &e, const_cast<MidiData&>(events))
		addMidiEvent(e);
	root->action_manager->endActionGroup();
}

void Track::deleteMidiEvent(int index)
{
	root->execute(new ActionTrackDeleteMidiEvent(this, index));
}

void Track::setName(const string& name)
{
	root->execute(new ActionTrackEditName(this, name));
}

void Track::setMuted(bool muted)
{
	root->execute(new ActionTrackEditMuted(this, muted));
}

void Track::setVolume(float volume)
{
	root->execute(new ActionTrackEditVolume(this, volume));
}

void Track::setPanning(float panning)
{
	root->execute(new ActionTrackEditPanning(this, panning));
}

void Track::insertMidiData(int offset, MidiData& midi)
{
	root->execute(new ActionTrackInsertMidi(this, offset, midi));
}

void Track::addEffect(Effect *effect)
{
	root->execute(new ActionTrackAddEffect(this, effect));
}

// execute after editing...
void Track::editEffect(int index, const string &param_old)
{
	root->execute(new ActionTrackEditEffect(this, index, param_old, fx[index]));
}

void Track::enableEffect(int index, bool enabled)
{
	if (fx[index]->enabled != enabled)
		root->execute(new ActionTrackToggleEffectEnabled(this, index));
}

void Track::deleteEffect(int index)
{
	root->execute(new ActionTrackDeleteEffect(this, index));
}

void Track::addMidiEffect(MidiEffect *effect)
{
	root->execute(new ActionTrackAddMidiEffect(this, effect));
}

// execute after editing...
void Track::editMidiEffect(int index, const string &param_old)
{
	root->execute(new ActionTrackEditMidiEffect(this, index, param_old, midi.fx[index]));
}

void Track::enableMidiEffect(int index, bool enabled)
{
	if (midi.fx[index]->enabled != enabled)
		root->execute(new ActionTrackToggleMidiEffectEnabled(this, index));
}

void Track::deleteMidiEffect(int index)
{
	root->execute(new ActionTrackDeleteMidiEffect(this, index));
}

void Track::setSynthesizer(Synthesizer *_synth)
{
	root->execute(new ActionTrackSetSynthesizer(this, _synth));
}

// execute after editing...
void Track::editSynthesizer(const string &param_old)
{
	root->execute(new ActionTrackEditSynthesizer(this, param_old));
}

void Track::addBar(int index, float bpm, int beats)
{
	BarPattern b;
	b.num_beats = beats;
	b.type = b.TYPE_BAR;
	b.length = (int)((float)b.num_beats * (float)root->sample_rate * 60.0f / bpm);
	if (index >= 0)
		root->execute(new ActionTrackAddBar(this, index + 1, b));
	else
		root->execute(new ActionTrackAddBar(this, bars.num, b));
}

void Track::addPause(int index, float time)
{
	BarPattern b;
	b.num_beats = 0;
	b.type = b.TYPE_PAUSE;
	b.length = (int)((float)root->sample_rate * time);
	if (index >= 0)
		root->execute(new ActionTrackAddBar(this, index + 1, b));
	else
		root->execute(new ActionTrackAddBar(this, bars.num, b));
}

void Track::editBar(int index, BarPattern &p)
{
	root->execute(new ActionTrackEditBar(this, index, p));
}

void Track::deleteBar(int index)
{
	root->execute(new ActionTrackDeleteBar(this, index));
}

void Track::addMarker(int pos, const string &text)
{
	root->execute(new ActionTrackAddMarker(this, pos, text));
}

void Track::deleteMarker(int index)
{
	root->execute(new ActionTrackDeleteMarker(this, index));
}

void Track::moveMarker(int index, int pos)
{
	root->execute(new ActionTrackMoveMarker(this, index, pos));
}



