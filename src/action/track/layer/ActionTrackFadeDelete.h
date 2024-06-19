/*
 * ActionTrackFadeDelete.h
 *
 *  Created on: 05.09.2018
 *      Author: michi
 */

#pragma once

#include "../../Action.h"
#include "../../../data/CrossFade.h"

namespace tsunami {

class TrackLayer;

class ActionTrackFadeDelete : public Action {
public:
	ActionTrackFadeDelete(TrackLayer *t, int index);

	string name() const override { return ":##:delete fade"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

	TrackLayer *layer;
	CrossFade fade;
	int index;
};

}
