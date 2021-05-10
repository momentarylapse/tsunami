/*
 * ActionCurveEdit.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionCurveEdit.h"
#include "../../Data/Track.h"
#include "../../Data/Curve.h"

ActionCurveEdit::ActionCurveEdit(Track *t, shared<Curve> _curve, const string &_name, float _min, float _max) {
	track = t;
	curve = _curve;
	name = _name;
	min = _min;
	max = _max;
}

void* ActionCurveEdit::execute(Data* d) {
	std::swap(name, curve->name);
	std::swap(min, curve->min);
	std::swap(max, curve->max);

	track->notify(track->MESSAGE_EDIT_CURVE);

	return nullptr;
}

void ActionCurveEdit::undo(Data* d) {
	execute(d);
}


