/*
 * ActionTrackEditPanning.cpp
 *
 *  Created on: 13.03.2013
 *      Author: michi
 */

#include "ActionTrackEditPanning.h"
#include "../../../data/Track.h"

ActionTrackEditPanning::ActionTrackEditPanning(Track *t, float _panning) {
	track = t;
	old_value = t->panning;
	new_value = _panning;
}

void *ActionTrackEditPanning::execute(Data *d) {
	track->panning = new_value;
	track->notify();

	return nullptr;
}

void ActionTrackEditPanning::undo(Data *d) {
	track->panning = old_value;
	track->notify();
}


bool ActionTrackEditPanning::mergable(Action *a) {
	auto *aa = dynamic_cast<ActionTrackEditPanning*>(a);
	if (!aa)
		return false;
	return (aa->track == track);
}

