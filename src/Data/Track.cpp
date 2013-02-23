/*
 * Track.cpp
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#include "Track.h"
#include "../Plugins/Effect.h"
#include "../Action/Track/Buffer/ActionTrackCreateBuffers.h"
#include "../Action/SubTrack/ActionTrackAddEmptySubTrack.h"
#include "../lib/hui/hui.h"
#include "../Action/Track/Data/ActionTrackEditName.h"
#include "../Action/Track/Data/ActionTrackEditMuted.h"
#include "../Action/Track/Data/ActionTrackEditVolume.h"
#include "../Action/Track/Midi/ActionTrackInsertMidi.h"


Track::Track()
{
	type = TYPE_AUDIO;
	muted = false;
	volume = 1;
	root = NULL;
	parent = -1;
	length = 0;
	pos = 0;
	is_selected = false;

	volume = 1;
	muted = false;


	rep_num = 0;
	rep_delay = 10000;

	area = rect(0, 0, 0, 0);
}



// destructor...
void Track::Reset()
{
	msg_db_r("Track.Reset",1);
	level.clear();
	name.clear();
	length = 0;
	pos = 0;
	area = rect(0, 0, 0, 0);
	volume = 1;
	muted = false;
	bar.clear();
	fx.clear();
	sub.clear();
	msg_db_l(1);
}

Track::~Track()
{
	Reset();
}


void SelectSub(Track *s, bool diff)
{
	if (!s)
		return;
	if (diff){
		s->is_selected = !s->is_selected;
	}else{
		if (!s->is_selected)
			s->root->UnselectAllSubs();

		// select this sub
		s->is_selected = true;
	}
}

void SelectTrack(Track *t, bool diff)
{
	if (!t)
		return;
	if (diff){
		bool is_only_selected = true;
		foreach(Track *tt, t->root->track)
			if ((tt->is_selected) && (tt != t))
				is_only_selected = false;
		t->is_selected = !t->is_selected || is_only_selected;
	}else{
		if (!t->is_selected){
			// unselect all tracks
			foreach(Track *tt, t->root->track)
				tt->is_selected = false;
		}

		// select this track
		t->is_selected = true;
	}
	t->root->UpdateSelection();
}

Track *Track::GetParent()
{
	if (parent >= 0)
		return root->track[parent];
	return NULL;
}

Range Track::GetRangeUnsafe()
{
	// sub
	if (parent >= 0)
		return Range(pos, length);

	int min =  1073741824;
	int max = -1073741824;
	foreach(TrackLevel &l, level)
		if (l.buffer.num > 0){
			min = min(l.buffer[0].offset, min);
			max = max(l.buffer.back().range().end(), max);
		}
	foreach(Track *s, sub){
		if (s->pos < min)
			min = s->pos;
		int smax = s->pos + s->length + s->rep_num * s->rep_delay;
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
	msg_db_r("Track.ReadBuffers", 1);

	// is <r> inside a buffer?
	foreach(BufferBox &b, level[level_no].buffer){
		int p0 = r.offset - b.offset;
		int p1 = r.offset - b.offset + r.num;
		if ((p0 >= 0) && (p1 <= b.num)){
			// set as reference to subarrays
			buf.set_as_ref(b, p0, p1 - p0);
			msg_db_l(1);
			return buf;
		}
	}

	// create own...
	buf.resize(r.num);

	// fill with overlapp
	foreach(BufferBox &b, level[level_no].buffer)
		buf.set(b, b.offset - r.offset, 1.0f);

	msg_db_l(1);
	return buf;
}

BufferBox Track::ReadBuffersCol(const Range &r)
{
	BufferBox buf;
	msg_db_r("Track.ReadBuffersCol", 1);

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
		msg_db_l(1);
		return buf;
	}

	// create own...
	buf.resize(r.num);

	// fill with overlapp
	foreach(TrackLevel &l, level)
		foreach(BufferBox &b, l.buffer)
			buf.add(b, b.offset - r.offset, 1.0f);

	msg_db_l(1);
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
	foreach(Track *s, sub)
		s->UpdatePeaks(mode);
}

void Track::InvalidateAllPeaks()
{
	foreach(TrackLevel &l, level)
		foreach(BufferBox &b, l.buffer)
			b.invalidate_peaks(b.range());
	foreach(Track *s, sub)
		s->InvalidateAllPeaks();
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

Track *Track::AddEmptySubTrack(const Range &r, const string &name)
{
	return (Track*)root->Execute(new ActionTrackAddEmptySubTrack(get_track_index(this), r, name));
}

void Track::InsertMidiData(int offset, MidiData& midi)
{
	root->Execute(new ActionTrackInsertMidi(this, offset, midi));
}



