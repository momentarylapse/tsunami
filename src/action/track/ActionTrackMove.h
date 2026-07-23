/*
 * ActionTrackMove.h
 *
 *  Created on: 28.08.2017
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

struct Track;

class ActionTrackMove: public history::Action {
public:
	ActionTrackMove(Track *track, int target);

	string name() const override { return ":##:move track"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	int origin, target;
};

}
