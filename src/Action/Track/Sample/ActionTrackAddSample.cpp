/*
 * ActionTrackAddSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionTrackAddSample.h"
#include "../../../Data/AudioFile.h"

ActionTrackAddSample::ActionTrackAddSample(int _track_no, int _pos, int _index)
{
	track_no = _track_no;
	pos = _pos;
	index = _index;
}

ActionTrackAddSample::~ActionTrackAddSample()
{
}

void ActionTrackAddSample::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->track[track_no];
	SampleRef *s = t->sample.pop();
	delete(s);
}



void *ActionTrackAddSample::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->track[track_no];
	SampleRef *s = new SampleRef(a->sample[index]);
	s->pos = pos;
	s->track_no = track_no;
	s->owner = a;
	t->sample.add(s);
	return s;
}

