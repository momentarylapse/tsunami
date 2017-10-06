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

class ActionCurveAddPoint : public Action
{
public:
	ActionCurveAddPoint(Curve *curve, int pos, float value);
	virtual ~ActionCurveAddPoint();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
private:
	Curve *curve;
	int index;
	int pos;
	float value;
};

#endif /* SRC_ACTION_CURVE_ACTIONCURVEADDPOINT_H_ */
