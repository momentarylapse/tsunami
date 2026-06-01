/*
 * ActionTrackAddCurve.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

class Curve;
class Track;

class ActionTrackAddCurve : public history::Action {
public:
	ActionTrackAddCurve(Track *t, shared<Curve> curve, int index);

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	shared<Curve> curve;
	Track *track;
	int index;
};

}
