/*
 * ActionCurveEdit.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionCurveEdit.h"
#include "../../Data/Song.h"
#include "../../Data/Curve.h"

ActionCurveEdit::ActionCurveEdit(shared<Curve> _curve, const string &_name, float _min, float _max, Array<Curve::Target> &_targets) {
	curve = _curve;
	name = _name;
	min = _min;
	max = _max;
	targets = _targets;
}

void* ActionCurveEdit::execute(Data* d) {
	Song *a = dynamic_cast<Song*>(d);

	auto _name = name;
	name = curve->name;
	curve->name = _name;

	auto _min = min;
	min = curve->min;
	curve->min = _min;

	auto _max = max;
	max = curve->max;
	curve->max = _max;

	auto _targets = targets;
	targets = curve->targets;
	curve->targets = _targets;

	a->notify(a->MESSAGE_EDIT_CURVE);

	return nullptr;
}

void ActionCurveEdit::undo(Data* d) {
	execute(d);
}


