/*
 * ActionTrackFadeAdd.h
 *
 *  Created on: 05.09.2018
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>
#include "../../../data/CrossFade.h"

namespace tsunami {

class TrackLayer;

class ActionTrackFadeAdd : public history::Action {
public:
	ActionTrackFadeAdd(TrackLayer *l, int position, CrossFade::Mode mode, int samples);

	string name() const override { return ":##:add fade"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	TrackLayer *layer;
	CrossFade fade;
	int index;
};

}
