/*
 * ActionTrackCurveAddPoint.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#pragma once

#include "../../Action.h"

namespace tsunami {

class Curve;

class ActionTrackCurveAddPoint : public Action {
public:
	ActionTrackCurveAddPoint(shared<Curve> curve, int pos, float value);

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	shared<Curve> curve;
	int index;
	int pos;
	float value;
};

}
