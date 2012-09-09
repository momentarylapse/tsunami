/*
 * Track.cpp
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#include "Track.h"
#include "../Action/Track/ActionTrackCreateBuffers.h"
#include "../Action/Track/ActionTrackAddEmptySubTrack.h"
#include "../lib/hui/hui.h"



void BarCollection::Update()
{
	length = 0;
	foreach(bar, b)
		length += b->length;
}

Track::Track()
{
	//HistoryStructReset("Track", this);

	type = TYPE_AUDIO;
	muted = false;
	volume = 1;
	root = NULL;
	parent = -1;
	length = 0;
	pos = 0;
	is_selected = false;
	is_mouse_over = false;
	cur_sub = -1;

	volume = 1;
	muted = false;


	rep_num = 0;
	rep_delay = 10000;

	x = 0;
	y = 0;
	width = 0;
	height = 0;
}



// destructor...
void Track::Reset()
{
	msg_db_r("Track.Reset",1);
	level.clear();
	name.clear();
	length = 0;
	pos = 0;
	width = -1;
	volume = 1;
	muted = false;
	bar_col.clear();
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
		foreach(t->root->track, tt)
			if ((tt->is_selected) && (&*tt != t))
				is_only_selected = false;
		t->is_selected = !t->is_selected || is_only_selected;
	}else{
		if (!t->is_selected){
			// unselect all tracks
			foreach(t->root->track, tt)
				tt->is_selected = false;
		}

		// select this track
		t->is_selected = true;
	}
	t->root->UpdateSelection();
}


Track *Track::GetCurSub()
{
	if ((cur_sub >= 0) && (cur_sub < sub.num))
		return &sub[cur_sub];
	return NULL;
}

Track *Track::GetParent()
{
	if (parent >= 0)
		return &root->track[parent];
	return NULL;
}

Range Track::GetRangeUnsafe()
{
	// sub
	if (parent >= 0)
		return Range(pos, length);

	int min = 2147483640;
	int max = -2147483640;
	foreach(level, l)
		if (l->buffer.num > 0){
			min = min(l->buffer[0].offset, min);
			max = max(l->buffer.back().offset + l->buffer.back().num, max);
		}
	foreach(sub, s){
		if (s->pos < min)
			min = s->pos;
		for (int i=0;i<s->rep_num+1;i++){
			int smax = s->pos + s->length + s->rep_num * s->rep_delay;
			if (smax > max)
				max = smax;
		}
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
	foreach(level[level_no].buffer, b){
		int p0 = r.offset - b->offset;
		int p1 = r.offset - b->offset + r.num;
		if ((p0 >= 0) && (p1 <= b->num)){
			// set as reference to subarrays
			buf.set_as_ref(*b, p0, p1 - p0);
			msg_db_l(1);
			return buf;
		}
	}

	// create own...
	buf.resize(r.num);

	// fill with overlapp
	foreach(level[level_no].buffer, b)
		buf.set(*b, b->offset - r.offset, 1.0f);

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
	foreachi(level, l, li)
		foreachi(l->buffer, b, bi){
			if (b->range().covers(r)){
				num_inside ++;
				inside_level = li;
				inside_no = bi;
				inside_p0 = r.offset - b->offset;
				inside_p1 = r.offset - b->offset + r.num;
			}else if (b->range().overlaps(r))
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
	foreach(level, l)
		foreach(l->buffer, b)
			buf.add(*b, b->offset - r.offset, 1.0f);

	msg_db_l(1);
	return buf;
}

BufferBox Track::GetBuffers(int level_no, const Range &r)
{
	root->Execute(new ActionTrackCreateBuffers(this, level_no, r));
	return ReadBuffers(level_no, r);
}

void Track::UpdatePeaks()
{
	foreach(level, l)
		foreach(l->buffer, b)
			b->update_peaks();
	foreach(sub, s)
		s->UpdatePeaks();
}

void Track::InvalidateAllPeaks()
{
	foreach(level, l)
		foreach(l->buffer, b)
			b->invalidate_peaks(b->range());
	foreach(sub, s)
		s->InvalidateAllPeaks();
}

Track *Track::AddEmptySubTrack(const Range &r, const string &name)
{
	return (Track*)root->Execute(new ActionTrackAddEmptySubTrack(get_track_index(this), r, name));
}


