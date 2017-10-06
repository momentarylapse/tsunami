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

class ActionCurveEditPoint : public ActionMergable<Curve::Point>
{
public:
	ActionCurveEditPoint(Curve *curve, int index, int pos, float value);
	virtual ~ActionCurveEditPoint();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
	virtual bool mergable(Action *a);
private:
	Curve *curve;
	int index;
};

#endif /* SRC_ACTION_CURVE_ACTIONCURVEEDITPOINT_H_ */
