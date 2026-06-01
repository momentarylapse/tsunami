/*
 * ActionTrackAdd.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

class Track;

class ActionTrackAdd : public history::Action {
public:
	ActionTrackAdd(Track *t, int index);

	string name() const override { return ":##:add track"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	shared<Track> track;
	int index;
};

}
