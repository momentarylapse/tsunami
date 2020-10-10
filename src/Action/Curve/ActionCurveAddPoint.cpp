/*
 * ActionCurveAddPoint.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionCurveAddPoint.h"
#include "../../Data/Song.h"
#include "../../Data/Curve.h"

ActionCurveAddPoint::ActionCurveAddPoint(shared<Curve> _curve, int _pos, float _value) {
	curve = _curve;
	index = curve->points.num;
	pos = _pos;
	value = _value;

	for (int i=0; i<curve->points.num; i++)
		if (pos < curve->points[i].pos) {
			index = i;
			break;
		}
}

void* ActionCurveAddPoint::execute(Data* d) {
	Curve::Point p;
	p.pos = pos;
	p.value = value;
	curve->points.insert(p, index);

	return nullptr;
}

void ActionCurveAddPoint::undo(Data* d) {
	curve->points.erase(index);
}

