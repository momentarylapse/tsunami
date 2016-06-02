/*
 * ActionTrackMoveSample.cpp
 *
 *  Created on: 30.03.2012
 *      Author: michi
 */

#include "ActionTrackMoveSample.h"

#include "../../../Data/Song.h"
#include "../../../Data/SongSelection.h"

ActionTrackMoveSample::ActionTrackMoveSample(Song *a, SongSelection &sel)
{
	foreachi(Track *t, a->tracks, ti)
		foreachi(SampleRef *s, t->samples, si)
			if (sel.has(s)){
				SubSaveData d;
				d.track_no = ti;
				d.sub_no = si;
				d.pos_old = s->pos;
				sub.add(d);
			}
	param = 0;
}



void *ActionTrackMoveSample::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	for (SubSaveData &d : sub)
		a->get_sample_ref(d.track_no, d.sub_no)->pos = d.pos_old + param;
	return NULL;
}



void ActionTrackMoveSample::abort(Data *d)
{
	undo(d);
}



void ActionTrackMoveSample::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	for (SubSaveData &d : sub)
		a->get_sample_ref(d.track_no, d.sub_no)->pos = d.pos_old;
}



void ActionTrackMoveSample::set_param_and_notify(Data *d, int _param)
{
	param += _param;
	execute(d);
	d->notify();
}

void ActionTrackMoveSample::abort_and_notify(Data *d)
{
	abort(d);
	d->notify();
}

bool ActionTrackMoveSample::is_trivial()
{
	return (param == 0);
}

