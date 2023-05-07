/*
 * ActionTrackEditVolume.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditVolume.h"
#include "../../../data/Track.h"

ActionTrackEditVolume::ActionTrackEditVolume(Track *t, float _volume) {
	track = t;
	old_value = t->volume;
	new_value = _volume;
}

void *ActionTrackEditVolume::execute(Data *d) {
	track->volume = new_value;
	track->out_changed.notify();

	return nullptr;
}

void ActionTrackEditVolume::undo(Data *d) {
	track->volume = old_value;
	track->out_changed.notify();
}


bool ActionTrackEditVolume::mergable(Action *a) {
	auto *aa = dynamic_cast<ActionTrackEditVolume*>(a);
	if (!aa)
		return false;
	return (aa->track == track);
}
