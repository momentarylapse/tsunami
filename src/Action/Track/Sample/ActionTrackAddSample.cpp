/*
 * ActionTrackAddSample.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionTrackAddSample.h"
#include "../../../Data/AudioFile.h"

ActionTrackAddSample::ActionTrackAddSample(Track *t, int _pos, int _index)
{
	track_no = get_track_index(t);
	pos = _pos;
	index = _index;
}

void ActionTrackAddSample::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->tracks[track_no];
	SampleRef *s = t->samples.pop();
	s->notify(s->MESSAGE_DELETE);
	delete(s);
}



void *ActionTrackAddSample::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->tracks[track_no];
	SampleRef *s = new SampleRef(a->samples[index]);
	s->pos = pos;
	s->track_no = track_no;
	s->owner = a;
	t->samples.add(s);
	return s;
}

