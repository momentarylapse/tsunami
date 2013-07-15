/*
 * ActionTrackMoveSample.cpp
 *
 *  Created on: 30.03.2012
 *      Author: michi
 */

#include "ActionTrackMoveSample.h"
#include "../../../Data/AudioFile.h"

ActionTrackMoveSample::ActionTrackMoveSample(AudioFile *a)
{
	foreach(Track *t, a->track)
		foreach(SampleRef *s, t->sample)
			if (s->is_selected){
				SubSaveData d;
				get_track_sub_index(s, d.track_no, d.sub_no);
				d.pos_old = s->pos;
				sub.add(d);
			}
	param = 0;
}



ActionTrackMoveSample::~ActionTrackMoveSample()
{
}



void *ActionTrackMoveSample::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	foreach(SubSaveData &d, sub)
		a->get_sub(d.track_no, d.sub_no)->pos = d.pos_old + param;
	return NULL;
}



void ActionTrackMoveSample::abort(Data *d)
{
	undo(d);
}



void ActionTrackMoveSample::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	foreach(SubSaveData &d, sub)
		a->get_sub(d.track_no, d.sub_no)->pos = d.pos_old;
}



void ActionTrackMoveSample::set_param_and_notify(Data *d, int _param)
{
	param += _param;
	execute(d);
	d->Notify("Change");
}

void ActionTrackMoveSample::abort_and_notify(Data *d)
{
	abort(d);
	d->Notify("Change");
}


