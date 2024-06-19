/*
 * ActionTrackEditName.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditName.h"
#include "../../../data/Track.h"

namespace tsunami {

ActionTrackEditName::ActionTrackEditName(Track *t, const string &_name) {
	track = t;
	old_value = t->name;
	new_value = _name;
}

void *ActionTrackEditName::execute(Data *d) {
	track->name = new_value;
	track->out_changed.notify();

	return nullptr;
}

void ActionTrackEditName::undo(Data *d) {
	track->name = old_value;
	track->out_changed.notify();
}


bool ActionTrackEditName::mergable(Action *a) {
	auto *aa = dynamic_cast<ActionTrackEditName*>(a);
	if (!aa)
		return false;
	return (aa->track == track);
}

}
