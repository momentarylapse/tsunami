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


Beat::Beat(int p, int bar, int beat)
{
	pos = p;
	bar_no = bar;
	beat_no = beat;
};

Array<Beat> BarCollection::GetBeats(const Range &r)
{
	Array<Beat> beats;

	int pos0 = 0;
	int bar_no = 0;
	foreach(Bar &b, *this)
		if (b.type == b.TYPE_BAR){
			for (int j=0;j<b.count;j++){
				for (int i=0;i<b.num_beats;i++){
					int pos = pos0 + i * b.length / b.num_beats;
					if (r.is_inside(pos))
						beats.add(Beat(pos, bar_no, i));
				}
				pos0 += b.length;
				bar_no ++;
			}
		}else if (b.type == b.TYPE_PAUSE){
			pos0 += b.length;
		}
	return beats;
}

int BarCollection::GetNextBeat(int pos)
{
	int p0 = 0;
	if (p0 > pos)
		return p0;
	foreach(Bar &b, *this){
		if (b.type == b.TYPE_BAR){
			for (int i=0;i<b.count;i++){
				int pp = p0;
				for (int j=0;j<b.num_beats;j++){
					pp += b.length / b.num_beats;
					if (pp > pos)
						return pp;
				}
				p0 += b.length;
			}
		}else if (b.type == b.TYPE_PAUSE){
			p0 += b.length;
			if (p0 > pos)
				return p0;
		}
	}
	return pos;
}

Range BarCollection::GetRange()
{
	int pos0 = 0;
	foreach(Bar &b, *this)
		if (b.type == b.TYPE_BAR){
			pos0 += b.length * b.count;
		}else if (b.type == b.TYPE_PAUSE){
			pos0 += b.length;
		}
	return Range(0, pos0);
}


MidiNote::MidiNote(const Range &_range, int _pitch, float _volume)
{
	range = _range;
	pitch = _pitch;
	volume = _volume;
}

float MidiNote::GetFrequency()
{
	return 440.0f * pow(2.0f, (float)(pitch - 69) / 12.0f);
}

Array<MidiNote> MidiData::GetNotes(const Range &r)
{
	Array<MidiNote> a;
	for (int i=0;i<num;i++)
		if ((*this)[i].range.overlaps(r))
			a.add((*this)[i]);
	return a;
}

int MidiData::GetNextNote(int pos)
{
	return 0;
}

Range MidiData::GetRange()
{
	if (num == 0)
		return Range(0, 0);
	int i0 = (*this)[0].range.offset;
	int i1 = back().range.end();
	return Range(i0, i1 - i0);
}


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

	int min = 2147483640;
	int max = -2147483640;
	foreach(TrackLevel &l, level)
		if (l.buffer.num > 0){
			min = min(l.buffer[0].offset, min);
			max = max(l.buffer.back().range().end(), max);
		}
	foreach(Track *s, sub){
		if (s->pos < min)
			min = s->pos;
		for (int i=0;i<s->rep_num+1;i++){
			int smax = s->pos + s->length + s->rep_num * s->rep_delay;
			if (smax > max)
				max = smax;
		}
	}

	if ((type == TYPE_TIME) && (bar.num > 0)){
		Range r = bar.GetRange();
		if (min > r.offset)
			min = r.offset;
		if (max < r.end())
			max = r.end();
	}

	return Range(min, max - min);
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


