/*
 * ActionCurveAdd.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionCurveAdd.h"
#include "../../Data/Track.h"
#include "../../Data/Curve.h"

ActionCurveAdd::ActionCurveAdd(Track *t, shared<Curve> _curve, int _index) {
	track = t;
	curve = _curve;
	index = _index;
}

void* ActionCurveAdd::execute(Data* d) {
	track->curves.insert(curve, index);
	track->notify(track->MESSAGE_ADD_CURVE);

	return curve.get();
}

void ActionCurveAdd::undo(Data* d) {
	curve->fake_death();
	track->curves.erase(index);

	track->notify(track->MESSAGE_DELETE_CURVE);
}


