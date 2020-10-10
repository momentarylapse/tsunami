/*
 * ActionCurveDelete.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#ifndef SRC_ACTION_CURVE_ACTIONCURVEDELETE_H_
#define SRC_ACTION_CURVE_ACTIONCURVEDELETE_H_

#include "../Action.h"

class Curve;

class ActionCurveDelete : public Action {
public:
	ActionCurveDelete(int index);

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	shared<Curve> curve;
	int index;
};

#endif /* SRC_ACTION_CURVE_ACTIONCURVEDELETE_H_ */
