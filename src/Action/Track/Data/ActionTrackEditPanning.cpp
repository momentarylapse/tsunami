/*
 * ActionTrackEditPanning.cpp
 *
 *  Created on: 13.03.2013
 *      Author: michi
 */

#include "ActionTrackEditPanning.h"
#include "../../../Data/Track.h"

ActionTrackEditPanning::ActionTrackEditPanning(Track *t, float _panning)
{
	track_no = get_track_index(t);
	old_value = t->panning;
	new_value = _panning;
}

ActionTrackEditPanning::~ActionTrackEditPanning()
{
}

void *ActionTrackEditPanning::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	t->panning = new_value;
	t->notify();

	return NULL;
}

void ActionTrackEditPanning::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	t->panning = old_value;
	t->notify();
}


bool ActionTrackEditPanning::mergable(Action *a)
{
	ActionTrackEditPanning *aa = dynamic_cast<ActionTrackEditPanning*>(a);
	if (!aa)
		return false;
	return (aa->track_no == track_no);
}

