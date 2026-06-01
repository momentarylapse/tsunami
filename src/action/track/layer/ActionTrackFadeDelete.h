/*
 * ActionTrackFadeDelete.h
 *
 *  Created on: 05.09.2018
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>
#include "../../../data/CrossFade.h"

namespace tsunami {

class TrackLayer;

class ActionTrackFadeDelete : public history::Action {
public:
	ActionTrackFadeDelete(TrackLayer *t, int index);

	string name() const override { return ":##:delete fade"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	TrackLayer *layer;
	CrossFade fade;
	int index;
};

}
