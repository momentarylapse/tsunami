/*
 * ActionTrackAddCurve.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionTrackAddCurve.h"
#include "../../../Data/Track.h"
#include "../../../Data/Curve.h"

ActionTrackAddCurve::ActionTrackAddCurve(Track *t, shared<Curve> _curve, int _index) {
	track = t;
	curve = _curve;
	index = _index;
}

void* ActionTrackAddCurve::execute(Data* d) {
	track->curves.insert(curve, index);
	track->notify(track->MESSAGE_ADD_CURVE);

	return curve.get();
}

void ActionTrackAddCurve::undo(Data* d) {
	curve->fake_death();
	track->curves.erase(index);

	track->notify(track->MESSAGE_DELETE_CURVE);
}


