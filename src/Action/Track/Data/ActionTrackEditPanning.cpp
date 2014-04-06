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
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	t->panning = new_value;
	t->Notify();

	return NULL;
}

void ActionTrackEditPanning::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	t->panning = old_value;
	t->Notify();
}


bool ActionTrackEditPanning::mergable(Action *a)
{
	ActionTrackEditPanning *aa = dynamic_cast<ActionTrackEditPanning*>(a);
	if (!aa)
		return false;
	return (aa->track_no == track_no);
}

