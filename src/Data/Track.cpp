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
#include "../Action/Track/Effect/ActionTrackAddEffect.h"
#include "../Action/Track/Effect/ActionTrackDeleteEffect.h"
#include "../Action/Track/Effect/ActionTrackEditEffect.h"
#include "../Action/Track/Effect/ActionTrackToggleEffectEnabled.h"
#include "../Action/Track/Marker/ActionTrackAddMarker.h"
#include "../Action/Track/Marker/ActionTrackDeleteMarker.h"
#include "../Action/Track/Marker/ActionTrackMoveMarker.h"
#include "../Action/Track/Midi/ActionTrackAddMidiNote.h"
#include "../Action/Track/Midi/ActionTrackDeleteMidiNote.h"


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

Array<string> get_instruments()
{
	Array<string> instruments;
	instruments.add("piano");
	instruments.add("organ");
	instruments.add("hapsichord");
	instruments.add("keyboard");
	instruments.add("guitar");
	instruments.add("drums");
	instruments.add("bass");
	instruments.add("vocals");
	instruments.add("violin");
	instruments.add("cello");
	instruments.add("flute");
	return instruments;
}

string get_instrument_name(const string &instrument)
{
	if (instrument == "piano")
		return _("Piano");
	if (instrument == "organ")
		return _("Orgel");
	if (instrument == "hapsichord")
		return _("Hapsichord");
	if (instrument == "keyboard")
		return _("Keyboard");
	if (instrument == "guitar")
		return _("Gitarre");
	if (instrument == "bass")
		return _("Bass");
	if (instrument == "drums")
		return _("Schlagzeug");
	if (instrument == "vocals")
		return _("Gesang");
	if (instrument == "violin")
		return _("Geige");
	if (instrument == "cello")
		return _("Cello");
	if (instrument == "flute")
		return _("Fl&ote");
	return "???";
}

int instrument_to_midi_no(const string &instrument)
{
	return 0;
}

string instrument_from_midi_no(int no)
{
	return "";
}

Array<int> get_default_tuning(const string &instrument)
{
	Array<int> tuning;
	if (instrument == "guitar"){
		tuning.add(40);
		tuning.add(45);
		tuning.add(50);
		tuning.add(55);
		tuning.add(59);
		tuning.add(64);
	}
	if (instrument == "bass"){
		tuning.add(28);
		tuning.add(33);
		tuning.add(38);
		tuning.add(43);
	}
	if (instrument == "cello"){
		tuning.add(36);
		tuning.add(43);
		tuning.add(50);
		tuning.add(57);
	}
	if (instrument == "violin"){
		tuning.add(55);
		tuning.add(62);
		tuning.add(69);
		tuning.add(76);
	}
	return tuning;
}

bool is_default_tuning(const string &instrument, const Array<int> &tuning)
{
	Array<int> def = get_default_tuning(instrument);
	if (def.num != tuning.num)
		return false;
	for (int i=0; i<tuning.num; i++)
		if (def[i] != tuning[i])
			return false;
	return true;
}

const string Track::MESSAGE_ADD_EFFECT = "AddEffect";
const string Track::MESSAGE_DELETE_EFFECT = "DeleteEffect";
const string Track::MESSAGE_ADD_MIDI_EFFECT = "AddMidiEffect";
const string Track::MESSAGE_DELETE_MIDI_EFFECT = "DeleteMidiEffect";
const Array<int> Track::DEFAULT_TUNING;

Track::Track() :
	Observable("Track")
{
	type = TYPE_AUDIO;
	muted = false;
	volume = 1;
	panning = 0;
	song = NULL;
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
	instrument.clear();
	tuning.clear();
	volume = 1;
	muted = false;
	panning = 0;
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
	if (song){
		foreachi(Track *t, song->tracks, i)
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
	song->execute(new ActionTrackCreateBuffers(this, level_no, r));
	return readBuffers(level_no, r);
}

void Track::updatePeaks()
{
	foreach(TrackLevel &l, levels)
		foreach(BufferBox &b, l.buffers)
			b.update_peaks();
}

void Track::invalidateAllPeaks()
{
	foreach(TrackLevel &l, levels)
		foreach(BufferBox &b, l.buffers)
			b.invalidate_peaks(b.range());
}

SampleRef *Track::addSample(int pos, int index)
{
	return (SampleRef*)song->execute(new ActionTrackAddSample(this, pos, index));
}

void Track::deleteSample(int index)
{
	song->execute(new ActionTrackDeleteSample(this, index));
}

void Track::editSample(int index, float volume, bool mute, int rep_num, int rep_delay)
{
	song->execute(new ActionTrackEditSample(this, index, volume, mute, rep_num, rep_delay));
}

void Track::addMidiNote(const MidiNote &n)
{
	song->execute(new ActionTrackAddMidiNote(this, n));
}

void Track::addMidiNotes(const MidiNoteData &midi)
{
	song->action_manager->beginActionGroup();
	foreach(MidiNote &n, const_cast<MidiNoteData&>(midi))
		addMidiNote(n);
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

void Track::setInstrument(const string& instrument, const Array<int> &tuning)
{
	song->execute(new ActionTrackSetInstrument(this, instrument, tuning));
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

void Track::insertMidiData(int offset, const MidiNoteData& midi)
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



