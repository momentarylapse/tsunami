/*
 * ActionTrackCurveEditPoint.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#pragma once

#include <lib/history/MergableAction.h>
#include "../../../data/Curve.h"

namespace tsunami {

//class Curve;

class ActionTrackCurveEditPoint : public history::MergableValueAction<Curve::Point> {
public:
	ActionTrackCurveEditPoint(shared<Curve> curve, int index, int pos, float value);

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;
	bool mergable(Action *a) override;
private:
	shared<Curve> curve;
	int index;
};

}
