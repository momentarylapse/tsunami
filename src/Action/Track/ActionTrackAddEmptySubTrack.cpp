/*
 * ActionTrackAddEmptySubTrack.cpp
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#include "ActionTrackAddEmptySubTrack.h"
#include "../../Data/AudioFile.h"

ActionTrackAddEmptySubTrack::ActionTrackAddEmptySubTrack(int _track_no, const Range &_range, const string &_name)
{
	track_no = _track_no;
	range = _range;
	name = _name;
}

ActionTrackAddEmptySubTrack::~ActionTrackAddEmptySubTrack()
{
}

void ActionTrackAddEmptySubTrack::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->track[track_no];

	Track *s = t->sub.pop();
	delete(s);
}



void *ActionTrackAddEmptySubTrack::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->track[track_no];

	Track *s = new Track;
	t->sub.add(s);

	s->name = name;
	s->pos = range.start();
	s->length = range.length();
	s->parent = get_track_index(t);
	s->root = a;
	s->level.resize(1);
	s->level[0].buffer.resize(1);
	s->level[0].buffer[0].offset = 0;
	s->level[0].buffer[0].resize(s->length);
	s->is_selected = true;

	if (a->GetCurTrack() == t)
		a->SetCurSub(s);
	return s;
}


