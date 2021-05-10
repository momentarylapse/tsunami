/*
 * ActionCurveDeletePoint.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionCurveDeletePoint.h"
#include "../../Data/Curve.h"


ActionCurveDeletePoint::ActionCurveDeletePoint(shared<Curve> _curve, int _index) {
	curve = _curve;
	index = _index;
	pos = 0;
	value = 0;
}

void* ActionCurveDeletePoint::execute(Data* d) {
	pos = curve->points[index].pos;
	value = curve->points[index].value;
	curve->points.erase(index);

	return nullptr;
}

void ActionCurveDeletePoint::undo(Data* d) {
	Curve::Point p;
	p.pos = pos;
	p.value = value;
	curve->points.insert(p, index);
}


