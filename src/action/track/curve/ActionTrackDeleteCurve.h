/*
 * ActionTrackDeleteCurve.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#pragma once

#include "../../Action.h"

namespace tsunami {

class Curve;
class Track;

class ActionTrackDeleteCurve : public Action {
public:
	ActionTrackDeleteCurve(Track *t, int index);

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Track *track;
	shared<Curve> curve;
	int index;
};

}
