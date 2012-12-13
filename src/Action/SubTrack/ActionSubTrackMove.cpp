/*
 * ActionSubTrackMove.cpp
 *
 *  Created on: 30.03.2012
 *      Author: michi
 */

#include "ActionSubTrackMove.h"
#include "../../Data/AudioFile.h"

ActionSubTrackMove::ActionSubTrackMove(AudioFile *a)
{
	foreach(Track *t, a->track)
		foreach(Track *s, t->sub)
			if (s->is_selected){
				SubSaveData d;
				get_track_sub_index(s, d.track_no, d.sub_no);
				d.pos_old = s->pos;
				sub.add(d);
			}
	param = 0;
}



ActionSubTrackMove::~ActionSubTrackMove()
{
}



void *ActionSubTrackMove::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	foreach(SubSaveData &d, sub)
		a->get_track(d.track_no, d.sub_no)->pos = d.pos_old + param;
	return NULL;
}



void ActionSubTrackMove::abort(Data *d)
{
	undo(d);
}



void ActionSubTrackMove::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	foreach(SubSaveData &d, sub)
		a->get_track(d.track_no, d.sub_no)->pos = d.pos_old;
}



void ActionSubTrackMove::set_param_and_notify(Data *d, int _param)
{
	param += _param;
	execute(d);
	d->Notify("Change");
}

void ActionSubTrackMove::abort_and_notify(Data *d)
{
	abort(d);
	d->Notify("Change");
}


