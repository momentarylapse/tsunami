/*
 * ActionCurveEditPoint.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#ifndef SRC_ACTION_CURVE_ACTIONCURVEEDITPOINT_H_
#define SRC_ACTION_CURVE_ACTIONCURVEEDITPOINT_H_

#include "../ActionMergable.h"
#include "../../Data/Curve.h"

//class Curve;

class ActionCurveEditPoint : public ActionMergable<Curve::Point> {
public:
	ActionCurveEditPoint(shared<Curve> curve, int index, int pos, float value);

	void *execute(Data *d) override;
	void undo(Data *d) override;
	bool mergable(Action *a) override;
private:
	shared<Curve> curve;
	int index;
};

#endif /* SRC_ACTION_CURVE_ACTIONCURVEEDITPOINT_H_ */
