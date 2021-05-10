/*
 * ActionTrackCurveEditPoint.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#pragma once

#include "../../ActionMergable.h"
#include "../../../Data/Curve.h"

//class Curve;

class ActionTrackCurveEditPoint : public ActionMergable<Curve::Point> {
public:
	ActionTrackCurveEditPoint(shared<Curve> curve, int index, int pos, float value);

	void *execute(Data *d) override;
	void undo(Data *d) override;
	bool mergable(Action *a) override;
private:
	shared<Curve> curve;
	int index;
};
