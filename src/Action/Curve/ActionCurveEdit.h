/*
 * ActionCurveEdit.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#ifndef SRC_ACTION_CURVE_ACTIONCURVEEDIT_H_
#define SRC_ACTION_CURVE_ACTIONCURVEEDIT_H_

#include "../Action.h"
#include "../../Data/Curve.h"

class Curve;

class ActionCurveEdit : public Action {
public:
	ActionCurveEdit(Curve *curve, const string &name, float min, float max, Array<Curve::Target> &targets);

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Curve *curve;
	string name;
	float min, max;
	Array<Curve::Target> targets;
};

#endif /* SRC_ACTION_CURVE_ACTIONCURVEEDIT_H_ */
