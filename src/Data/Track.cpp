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
#include "../Action/Track/Midi/ActionTrackAddMidiNote.h"
#include "../Action/Track/Midi/ActionTrackDeleteMidiNote.h"
#include "../Action/Track/Midi/ActionTrackInsertMidi.h"
#include "../Action/Track/Sample/ActionTrackAddSample.h"
#include "../Action/Track/Sample/ActionTrackDeleteSample.h"
#include "../Action/Track/Synthesizer/ActionTrackSetSynthesizer.h"
#include "../Action/Track/Effect/ActionTrackAddEffect.h"
#include "../Action/Track/Effect/ActionTrackDeleteEffect.h"
#include "../Action/Track/Effect/ActionTrackEditEffect.h"
#include "../Action/Track/Bar/ActionTrackAddBar.h"
#include "../Action/Track/Bar/ActionTrackEditBar.h"
#include "../Action/Track/Bar/ActionTrackDeleteBar.h"


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

	area = rect(0, 0, 0, 0);
}



// destructor...
void Track::Reset()
{
	msg_db_f("Track.Reset",1);
	level.clear();
	name.clear();
	area = rect(0, 0, 0, 0);
	volume = 1;
	muted = false;
	panning = 0;
	bar.clear();
	foreach(Effect *f, fx)
		delete(f);
	fx.clear();
	sample.clear();
	if (synth)
		delete(synth);
	synth = CreateSynthesizer("Dummy");
}

Track::~Track()
{
	Notify("Delete");
	Reset();
	if (synth)
		delete(synth);
}

Range Track::GetRangeUnsafe()
{
	int min =  1073741824;
	int max = -1073741824;
	foreach(TrackLevel &l, level)
		if (l.buffer.num > 0){
			min = min(l.buffer[0].offset, min);
			max = max(l.buffer.back().range().end(), max);
		}
	foreach(SampleRef *s, sample){
		if (s->pos < min)
			min = s->pos;
		int smax = s->pos + s->buf.num + s->rep_num * s->rep_delay;
		if (smax > max)
			max = smax;
	}
	Range r = Range(min, max - min);

	if ((type == TYPE_TIME) && (bar.num > 0))
		r = r || bar.GetRange();

	if ((type == TYPE_MIDI) && (midi.num > 0))
		r = r || midi.GetRange();

	return r;
}

Range Track::GetRange()
{
	Range r = GetRangeUnsafe();
	if (r.length() < 0)
		return Range(0, 0);
	return r;
}

string Track::GetNiceName()
{
	if (name.num > 0)
		return name;
	return _("namenlose Spur");
}

BufferBox Track::ReadBuffers(int level_no, const Range &r)
{
	BufferBox buf;
	msg_db_f("Track.ReadBuffers", 1);

	// is <r> inside a buffer?
	foreach(BufferBox &b, level[level_no].buffer){
		int p0 = r.offset - b.offset;
		int p1 = r.offset - b.offset + r.num;
		if ((p0 >= 0) && (p1 <= b.num)){
			// set as reference to subarrays
			buf.set_as_ref(b, p0, p1 - p0);
			return buf;
		}
	}

	// create own...
	buf.resize(r.num);

	// fill with overlapp
	foreach(BufferBox &b, level[level_no].buffer)
		buf.set(b, b.offset - r.offset, 1.0f);

	return buf;
}

BufferBox Track::ReadBuffersCol(const Range &r)
{
	BufferBox buf;
	msg_db_f("Track.ReadBuffersCol", 1);

	// is <r> inside a single buffer?
	int num_inside = 0;
	int inside_level, inside_no;
	int inside_p0, inside_p1;
	bool intersected = false;
	foreachi(TrackLevel &l, level, li)
		foreachi(BufferBox &b, l.buffer, bi){
			if (b.range().covers(r)){
				num_inside ++;
				inside_level = li;
				inside_no = bi;
				inside_p0 = r.offset - b.offset;
				inside_p1 = r.offset - b.offset + r.num;
			}else if (b.range().overlaps(r))
				intersected = true;
		}
	if ((num_inside == 1) && (!intersected)){
		// set as reference to subarrays
		buf.set_as_ref(level[inside_level].buffer[inside_no], inside_p0, inside_p1 - inside_p0);
		return buf;
	}

	// create own...
	buf.resize(r.num);

	// fill with overlapp
	foreach(TrackLevel &l, level)
		foreach(BufferBox &b, l.buffer)
			buf.add(b, b.offset - r.offset, 1.0f, 0.0f);

	return buf;
}

BufferBox Track::GetBuffers(int level_no, const Range &r)
{
	root->Execute(new ActionTrackCreateBuffers(this, level_no, r));
	return ReadBuffers(level_no, r);
}

void Track::UpdatePeaks(int mode)
{
	foreach(TrackLevel &l, level)
		foreach(BufferBox &b, l.buffer)
			b.update_peaks(mode);
}

void Track::InvalidateAllPeaks()
{
	foreach(TrackLevel &l, level)
		foreach(BufferBox &b, l.buffer)
			b.invalidate_peaks(b.range());
}

SampleRef *Track::AddSample(int pos, int index)
{
	return (SampleRef*)root->Execute(new ActionTrackAddSample(this, pos, index));
}

void Track::DeleteSample(int index)
{
	root->Execute(new ActionTrackDeleteSample(this, index));
}

void Track::AddMidiNote(const MidiNote &n)
{
	root->Execute(new ActionTrackAddMidiNote(this, n));
}

void Track::DeleteMidiNote(int index)
{
	root->Execute(new ActionTrackDeleteMidiNote(this, index));
}

void Track::SetName(const string& name)
{
	root->Execute(new ActionTrackEditName(this, name));
}

void Track::SetMuted(bool muted)
{
	root->Execute(new ActionTrackEditMuted(this, muted));
}

void Track::SetVolume(float volume)
{
	root->Execute(new ActionTrackEditVolume(this, volume));
}

void Track::SetPanning(float panning)
{
	root->Execute(new ActionTrackEditPanning(this, panning));
}

void Track::InsertMidiData(int offset, MidiData& midi)
{
	root->Execute(new ActionTrackInsertMidi(this, offset, midi));
}

void Track::AddEffect(Effect *effect)
{
	root->Execute(new ActionTrackAddEffect(this, effect));
}

// execute after editing...
void Track::EditEffect(int index, const string &param_old)
{
	root->Execute(new ActionTrackEditEffect(this, index, param_old));
}

void Track::DeleteEffect(int index)
{
	root->Execute(new ActionTrackDeleteEffect(this, index));
}

void Track::SetSynthesizer(Synthesizer *_synth)
{
	root->Execute(new ActionTrackSetSynthesizer(this, _synth));
}

void Track::AddBars(int index, float bpm, int beats, int bars)
{
	BarPattern b;
	b.num_beats = beats;
	b.type = b.TYPE_BAR;
	b.length = (int)((float)b.num_beats * (float)root->sample_rate * 60.0f / bpm);
	b.count = bars;
	if (index >= 0)
		root->Execute(new ActionTrackAddBar(this, index + 1, b));
	else
		root->Execute(new ActionTrackAddBar(this, bar.num, b));
}

void Track::AddPause(int index, float time)
{
	BarPattern b;
	b.num_beats = 1;
	b.type = b.TYPE_PAUSE;
	b.length = (int)((float)root->sample_rate * time);
	b.count = 1;
	if (index >= 0)
		root->Execute(new ActionTrackAddBar(this, index + 1, b));
	else
		root->Execute(new ActionTrackAddBar(this, bar.num, b));
}

void Track::EditBar(int index, BarPattern &p)
{
	root->Execute(new ActionTrackEditBar(this, index, p));
}

void Track::DeleteBar(int index)
{
	root->Execute(new ActionTrackDeleteBar(this, index));
}



