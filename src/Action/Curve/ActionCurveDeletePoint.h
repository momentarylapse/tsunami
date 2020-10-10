/*
 * ActionCurveDeletePoint.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#ifndef SRC_ACTION_CURVE_ACTIONCURVEDELETEPOINT_H_
#define SRC_ACTION_CURVE_ACTIONCURVEDELETEPOINT_H_

#include "../Action.h"

class Curve;

class ActionCurveDeletePoint : public Action {
public:
	ActionCurveDeletePoint(shared<Curve> curve, int index);

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	shared<Curve> curve;
	int index;
	int pos;
	float value;
};

#endif /* SRC_ACTION_CURVE_ACTIONCURVEDELETEPOINT_H_ */
