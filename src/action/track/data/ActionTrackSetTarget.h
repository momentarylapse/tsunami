/*
 * ActionTrackSetTarget.h
 *
 *  Created on: 18.06.2019
 *      Author: michi
 */

#pragma once

#include "../../Action.h"

class Track;

class ActionTrackSetTarget : public Action {
public:
	ActionTrackSetTarget(Track *t, Track *target);

	string name() const override { return ":##:set track target"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Track *track;
	Track *target;
};
