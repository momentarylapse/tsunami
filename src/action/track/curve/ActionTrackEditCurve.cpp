/*
 * ActionTrackEditCurve.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionTrackEditCurve.h"
#include "../../../data/Track.h"
#include "../../../data/Curve.h"

ActionTrackEditCurve::ActionTrackEditCurve(Track *t, shared<Curve> _curve, const string &_name, float _min, float _max, CurveType _type) {
	track = t;
	curve = _curve;
	name = _name;
	min = _min;
	max = _max;
	type = _type;
}

void* ActionTrackEditCurve::execute(Data* d) {
	std::swap(name, curve->name);
	std::swap(min, curve->min);
	std::swap(max, curve->max);
	std::swap(type, curve->type);

	track->out_edit_curve.notify();
	return nullptr;
}

void ActionTrackEditCurve::undo(Data* d) {
	execute(d);
}


