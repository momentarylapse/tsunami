/*
 * ActionCurveAdd.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#ifndef SRC_ACTION_CURVE_ACTIONCURVEADD_H_
#define SRC_ACTION_CURVE_ACTIONCURVEADD_H_

#include "../Action.h"

class Curve;

class ActionCurveAdd : public Action
{
public:
	ActionCurveAdd(Curve *curve, int index);
	virtual ~ActionCurveAdd();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
private:
	Curve *curve;
	int index;
};

#endif /* SRC_ACTION_CURVE_ACTIONCURVEADD_H_ */
