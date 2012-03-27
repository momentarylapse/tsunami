/*
 * Track.cpp
 *
 *  Created on: 22.03.2012
 *      Author: michi
 */

#include "Track.h"
#include "../Action/ActionTrackCreateBuffers.h"
#include "../Action/ActionTrackAddEmptySubTrack.h"



void BarCollection::Update()
{
	length = 0;
	foreach(bar, b)
		length += b.length;
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
	buffer.clear();
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
			if ((tt.is_selected) && (&tt != t))
				is_only_selected = false;
		t->is_selected = !t->is_selected || is_only_selected;
	}else{
		if (!t->is_selected){
			// unselect all tracks
			foreach(t->root->track, tt)
				tt.is_selected = false;
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

int Track::GetMinUnsafe()
{
	int min = 2147483640;
	if (buffer.num > 0)
		min = buffer[0].offset;
	foreachc(sub, s)
		if (s.pos < min)
			min = s.pos;
	return min;
}

int Track::GetMaxUnsafe()
{
	int max = -2147483640;
	if (buffer.num > 0)
		max = buffer.back().offset + buffer.back().num;
	foreachc(sub, s)
		for (int i=0;i<s.rep_num+1;i++){
			int smax = s.pos + s.length + s.rep_num * s.rep_delay;
			if (smax > max)
				max = smax;
		}
	return max;
}

int Track::GetMin()
{
	int min = GetMinUnsafe();
	if (min == 2147483640)
		return 0;
	return min;
}

int Track::GetMax()
{
	int max = GetMaxUnsafe();
	if (max == -2147483640)
		return 0;
	return max;
}

string Track::GetNiceName()
{
	return i2s(get_track_index(this) + 1) + ": " + name;
}

BufferBox Track::ReadBuffers(int pos, int length)
{
	BufferBox buf;
	msg_db_r("Track.ReadBuffers", 1);

	// is <pos..length> inside a buffer?
	foreach(buffer, b){
		int p0 = pos - b.offset;
		int p1 = pos - b.offset + length;
		if ((p0 >= 0) && (p1 <= b.num)){
			// set as reference to subarrays
			buf.set_as_ref(b, p0, p1 - p0);
			msg_db_l(1);
			return buf;
		}
	}

	// create own...
	buf.resize(length);

	// fill with overlapp
	foreach(buffer, b)
		buf.set(b, b.offset - pos, 1.0f);

	msg_db_l(1);
	return buf;
}

BufferBox Track::GetBuffers(int pos, int length)
{
	root->Execute(new ActionTrackCreateBuffers(this, pos, length));
	return ReadBuffers(pos, length);
}

void Track::UpdatePeaks()
{}

Track *Track::AddEmptySubTrack(int pos, int length, const string &name)
{
	return (Track*)root->Execute(new ActionTrackAddEmptySubTrack(get_track_index(this), pos, length, name));
}


