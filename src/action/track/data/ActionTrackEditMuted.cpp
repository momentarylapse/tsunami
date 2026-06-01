/*
 * ActionTrackEditMuted.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditMuted.h"
#include "../../../data/Track.h"

namespace tsunami {

ActionTrackEditMuted::ActionTrackEditMuted(Track *t, bool _muted) {
	track = t;
	muted = _muted;
}

void *ActionTrackEditMuted::execute(history::Data *d) {
	bool temp = muted;
	muted = track->muted;
	track->muted = temp;
	track->out_changed.notify();

	return nullptr;
}

void ActionTrackEditMuted::undo(history::Data *d) {
	execute(d);
}

}
