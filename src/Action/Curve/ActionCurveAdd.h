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
class Track;

class ActionCurveAdd : public Action {
public:
	ActionCurveAdd(Track *t, shared<Curve> curve, int index);

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	shared<Curve> curve;
	Track *track;
	int index;
};

#endif /* SRC_ACTION_CURVE_ACTIONCURVEADD_H_ */
