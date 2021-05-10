/*
 * ActionTrackCurveDeletePoint.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#pragma once

#include "../../Action.h"

class Curve;

class ActionTrackCurveDeletePoint : public Action {
public:
	ActionTrackCurveDeletePoint(shared<Curve> curve, int index);

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	shared<Curve> curve;
	int index;
	int pos;
	float value;
};
