/*
 * ActionCurveAddPoint.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#ifndef SRC_ACTION_CURVE_ACTIONCURVEADDPOINT_H_
#define SRC_ACTION_CURVE_ACTIONCURVEADDPOINT_H_

#include "../Action.h"

class Curve;

class ActionCurveAddPoint : public Action {
public:
	ActionCurveAddPoint(Curve *curve, int pos, float value);

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Curve *curve;
	int index;
	int pos;
	float value;
};

#endif /* SRC_ACTION_CURVE_ACTIONCURVEADDPOINT_H_ */
