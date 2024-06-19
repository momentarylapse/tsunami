/*
 * ActionTrackCurveEditPoint.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionTrackCurveEditPoint.h"
#include "../../../data/Curve.h"

namespace tsunami {

ActionTrackCurveEditPoint::ActionTrackCurveEditPoint(shared<Curve> _curve, int _index, int _pos, float _value) {
	curve = _curve;
	index = _index;
	new_value.pos = _pos;
	new_value.value = _value;
}

void* ActionTrackCurveEditPoint::execute(Data* d) {
	old_value = curve->points[index];
	curve->points[index] = new_value;

	return nullptr;
}

void ActionTrackCurveEditPoint::undo(Data* d) {
	curve->points[index] = old_value;
}

bool ActionTrackCurveEditPoint::mergable(Action *a) {
	auto *e = dynamic_cast<ActionTrackCurveEditPoint*>(a);
	if (!e)
		return false;
	return (curve == e->curve.get()) and (index == e->index);
}

}

