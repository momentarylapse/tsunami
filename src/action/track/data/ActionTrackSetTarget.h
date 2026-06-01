/*
 * ActionTrackSetTarget.h
 *
 *  Created on: 18.06.2019
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

class Track;

class ActionTrackSetTarget : public history::Action {
public:
	ActionTrackSetTarget(Track *t, Track *target);

	string name() const override { return ":##:set track target"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	Track *track;
	Track *target;
};

}
