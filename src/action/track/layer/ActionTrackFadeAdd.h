/*
 * ActionTrackFadeAdd.h
 *
 *  Created on: 05.09.2018
 *      Author: michi
 */

#pragma once

#include "../../Action.h"
#include "../../../data/CrossFade.h"

namespace tsunami {

class TrackLayer;

class ActionTrackFadeAdd : public Action {
public:
	ActionTrackFadeAdd(TrackLayer *l, int position, CrossFade::Mode mode, int samples);

	string name() const override { return ":##:add fade"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

	TrackLayer *layer;
	CrossFade fade;
	int index;
};

}
