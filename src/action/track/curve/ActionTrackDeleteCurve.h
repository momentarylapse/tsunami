/*
 * ActionTrackDeleteCurve.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>

namespace tsunami {

class Curve;
class Track;

class ActionTrackDeleteCurve : public history::Action {
public:
	ActionTrackDeleteCurve(Track *t, int index);

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	Track *track;
	shared<Curve> curve;
	int index;
};

}
