/*
 * ActionTrackCurveAddPoint.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionTrackCurveAddPoint.h"
#include "../../../data/Song.h"
#include "../../../data/Curve.h"

namespace tsunami {

ActionTrackCurveAddPoint::ActionTrackCurveAddPoint(shared<Curve> _curve, int _pos, float _value) {
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

void* ActionTrackCurveAddPoint::execute(Data* d) {
	Curve::Point p;
	p.pos = pos;
	p.value = value;
	curve->points.insert(p, index);

	return nullptr;
}

void ActionTrackCurveAddPoint::undo(Data* d) {
	curve->points.erase(index);
}

}

