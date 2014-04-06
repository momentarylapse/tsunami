/*
 * ActionTrackEditSample.cpp
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#include "ActionTrackEditSample.h"
#include "../../../Data/AudioFile.h"

ActionTrackEditSample::ActionTrackEditSample(Track *t, int _index, float volume, bool mute, int rep_num, int rep_delay)
{
	track_no = get_track_index(t);
	index = _index;
	SampleRef *s = t->sample[index];
	old_value.volume = s->volume;
	old_value.mute = s->muted;
	old_value.rep_num = s->rep_num;
	old_value.rep_delay = s->rep_delay;
	new_value.volume = volume;
	new_value.mute = mute;
	new_value.rep_num = rep_num;
	new_value.rep_delay = rep_delay;
}

ActionTrackEditSample::~ActionTrackEditSample()
{
}

void *ActionTrackEditSample::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);
	SampleRef *s = t->sample[index];

	s->volume = new_value.volume;
	s->muted = new_value.mute;
	s->rep_num = new_value.rep_num;
	s->rep_delay = new_value.rep_delay;
	s->Notify(s->MESSAGE_CHANGE_BY_ACTION);

	return NULL;
}

void ActionTrackEditSample::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);
	SampleRef *s = t->sample[index];

	s->volume = old_value.volume;
	s->muted = old_value.mute;
	s->rep_num = old_value.rep_num;
	s->rep_delay = old_value.rep_delay;
	s->Notify(s->MESSAGE_CHANGE_BY_ACTION);
}


bool ActionTrackEditSample::mergable(Action *a)
{
	ActionTrackEditSample *aa = dynamic_cast<ActionTrackEditSample*>(a);
	if (!aa)
		return false;
	return (aa->track_no == track_no) && (aa->index == index);
}

