/*
 * ActionTrackAddEmptySubTrack.cpp
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#include "ActionTrackAddEmptySubTrack.h"
#include "../Data/AudioFile.h"

ActionTrackAddEmptySubTrack::ActionTrackAddEmptySubTrack(int _track_no, int _pos, int _length, const string &_name)
{
	track_no = _track_no;
	pos = _pos;
	length = _length;
	name = _name;
}

ActionTrackAddEmptySubTrack::~ActionTrackAddEmptySubTrack()
{
}

void ActionTrackAddEmptySubTrack::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track &t = a->track[track_no];

	t.sub.pop();
}



void *ActionTrackAddEmptySubTrack::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track &t = a->track[track_no];

	Track ss;
	t.sub.add(ss);

	Track *s = &t.sub.back();
	s->name = name;
	s->pos = pos;
	s->length = length;
	s->parent = get_track_index(&t);
	s->root = a;

	if (a->GetCurTrack() == &t)
		a->SetCurSub(s);
	return s;
}



void ActionTrackAddEmptySubTrack::redo(Data *d)
{
	execute(d);
}


