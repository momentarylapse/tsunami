/*
 * ActionTrackAddCurve.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#pragma once

#include "../../Action.h"

class Curve;
class Track;

class ActionTrackAddCurve : public Action {
public:
	ActionTrackAddCurve(Track *t, shared<Curve> curve, int index);

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	shared<Curve> curve;
	Track *track;
	int index;
};
