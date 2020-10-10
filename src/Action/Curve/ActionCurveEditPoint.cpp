/*
 * ActionCurveEditPoint.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionCurveEditPoint.h"
#include "../../Data/Song.h"
#include "../../Data/Curve.h"

ActionCurveEditPoint::ActionCurveEditPoint(shared<Curve> _curve, int _index, int _pos, float _value) {
	curve = _curve;
	index = _index;
	new_value.pos = _pos;
	new_value.value = _value;
}

void* ActionCurveEditPoint::execute(Data* d) {
	old_value = curve->points[index];
	curve->points[index] = new_value;

	return nullptr;
}

void ActionCurveEditPoint::undo(Data* d) {
	curve->points[index] = old_value;
}

bool ActionCurveEditPoint::mergable(Action *a) {
	auto *e = dynamic_cast<ActionCurveEditPoint*>(a);
	if (!e)
		return false;
	return (curve == e->curve.get()) and (index == e->index);

}

