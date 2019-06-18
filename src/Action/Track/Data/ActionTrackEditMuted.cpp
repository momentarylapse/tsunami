/*
 * ActionTrackEditMuted.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditMuted.h"
#include "../../../Data/Track.h"

ActionTrackEditMuted::ActionTrackEditMuted(Track *t, bool _muted) {
	track = t;
	muted = _muted;
}

void *ActionTrackEditMuted::execute(Data *d) {
	bool temp = muted;
	muted = track->muted;
	track->muted = temp;
	track->notify();

	return nullptr;
}

void ActionTrackEditMuted::undo(Data *d) {
	execute(d);
}
