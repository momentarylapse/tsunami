/*
 * ActionTrackAddSampleRef.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionTrackAddSampleRef.h"
#include "../../../Data/AudioFile.h"

ActionTrackAddSampleRef::ActionTrackAddSampleRef(int _track_no, int _pos, int _index)
{
	track_no = _track_no;
	pos = _pos;
	index = _index;
}

ActionTrackAddSampleRef::~ActionTrackAddSampleRef()
{
}

void ActionTrackAddSampleRef::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->track[track_no];
	SampleRef *s = t->sample.pop();
	delete(s);
}



void *ActionTrackAddSampleRef::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->track[track_no];
	SampleRef *s = new SampleRef(a->sample[index]);
	s->pos = pos;
	s->parent = track_no;
	s->root = a;
	t->sample.add(s);
	return s;
}

