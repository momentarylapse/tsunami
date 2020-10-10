/*
 * ActionCurveAdd.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionCurveAdd.h"
#include "../../Data/Song.h"
#include "../../Data/Curve.h"

ActionCurveAdd::ActionCurveAdd(shared<Curve> _curve, int _index) {
	curve = _curve;
	index = _index;
}

void* ActionCurveAdd::execute(Data* d) {
	Song *a = dynamic_cast<Song*>(d);

	a->curves.insert(curve, index);
	a->notify(a->MESSAGE_ADD_CURVE);

	return curve.get();
}

void ActionCurveAdd::undo(Data* d) {
	Song *a = dynamic_cast<Song*>(d);

	curve->fake_death();
	a->curves.erase(index);

	a->notify(a->MESSAGE_DELETE_CURVE);
}


