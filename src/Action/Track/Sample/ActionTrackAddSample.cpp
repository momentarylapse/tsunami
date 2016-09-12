/*
 * ActionTrackAddSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionTrackAddSample.h"

#include "../../../Data/Song.h"

ActionTrackAddSample::ActionTrackAddSample(Track *t, int _pos, Sample *_sample)
{
	track_no = get_track_index(t);
	pos = _pos;
	sample = _sample;
}

void ActionTrackAddSample::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->tracks[track_no];
	SampleRef *s = t->samples.pop();
	s->notify(s->MESSAGE_DELETE);
	delete(s);
}



void *ActionTrackAddSample::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->tracks[track_no];
	SampleRef *s = new SampleRef(sample);
	s->pos = pos;
	s->track_no = track_no;
	s->owner = a;
	t->samples.add(s);
	return s;
}

