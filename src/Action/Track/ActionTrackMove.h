/*
 * ActionTrackMove.h
 *
 *  Created on: 28.08.2017
 *      Author: michi
 */

#pragma once

#include "../Action.h"

class Track;

class ActionTrackMove: public Action {
public:
	ActionTrackMove(Track *track, int target);

	string name() const override { return ":##:move track"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

	int origin, target;
};
