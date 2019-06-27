/*
 * ActionCurveDelete.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionCurveDelete.h"
#include "../../Data/Song.h"
#include "../../Data/Curve.h"

ActionCurveDelete::ActionCurveDelete(int _index) {
	curve = nullptr;
	index = _index;
}

ActionCurveDelete::~ActionCurveDelete() {
	if (curve)
		delete curve;
}

void* ActionCurveDelete::execute(Data* d) {
	Song *a = dynamic_cast<Song*>(d);

	curve = a->curves[index];
	curve->fake_death();
	a->curves.erase(index);

	a->notify(a->MESSAGE_DELETE_CURVE);

	return nullptr;
}

void ActionCurveDelete::undo(Data* d) {
	Song *a = dynamic_cast<Song*>(d);

	a->curves.insert(curve, index);
	a->notify(a->MESSAGE_ADD_CURVE);
	curve = nullptr;
}

