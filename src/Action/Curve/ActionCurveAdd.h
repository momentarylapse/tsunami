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

class ActionCurveAdd : public Action {
public:
	ActionCurveAdd(Curve *curve, int index);
	~ActionCurveAdd();

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Curve *curve;
	int index;
};

#endif /* SRC_ACTION_CURVE_ACTIONCURVEADD_H_ */
