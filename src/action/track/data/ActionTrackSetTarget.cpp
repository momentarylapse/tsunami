/*
 * ActionTrackSetTarget.cpp
 *
 *  Created on: 18.06.2019
 *      Author: michi
 */

#include "ActionTrackSetTarget.h"
#include "../../../data/Track.h"

namespace tsunami {

ActionTrackSetTarget::ActionTrackSetTarget(Track *t, Track *_target) {
	track = t;
	target = _target;
}

void *ActionTrackSetTarget::execute(history::Data* d) {
	std::swap(track->send_target, target);
	track->out_changed.notify();

	return nullptr;
}

void ActionTrackSetTarget::undo(history::Data* d) {
	execute(d);
}

}
