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
	ActionCurveEdit(Track *t, shared<Curve> curve, const string &name, float min, float max);

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Track *track;
	shared<Curve> curve;
	string name;
	float min, max;
};

#endif /* SRC_ACTION_CURVE_ACTIONCURVEEDIT_H_ */
