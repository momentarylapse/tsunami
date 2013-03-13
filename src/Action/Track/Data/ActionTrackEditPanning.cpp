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
	panning = _panning;
}

ActionTrackEditPanning::~ActionTrackEditPanning()
{
}

void *ActionTrackEditPanning::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no, -1);

	float temp = panning;
	panning = t->panning;
	t->panning = temp;

	return NULL;
}

void ActionTrackEditPanning::undo(Data *d)
{
	execute(d);
}

