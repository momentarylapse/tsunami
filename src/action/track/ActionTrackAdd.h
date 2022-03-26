/*
 * ActionTrackAdd.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#pragma once

#include "../Action.h"

class Track;

class ActionTrackAdd : public Action {
public:
	ActionTrackAdd(Track *t, int index);

	string name() const override { return ":##:add track"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	shared<Track> track;
	int index;
};
