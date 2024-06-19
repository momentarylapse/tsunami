/*
 * ActionTrackAddCurve.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionTrackAddCurve.h"
#include "../../../data/Track.h"
#include "../../../data/Curve.h"

namespace tsunami {

ActionTrackAddCurve::ActionTrackAddCurve(Track *t, shared<Curve> _curve, int _index) {
	track = t;
	curve = _curve;
	index = _index;
}

void* ActionTrackAddCurve::execute(Data* d) {
	track->curves.insert(curve, index);
	track->out_curve_list_changed.notify();

	return curve.get();
}

void ActionTrackAddCurve::undo(Data* d) {
	curve->fake_death();
	track->curves.erase(index);

	track->out_curve_list_changed.notify();
}

}


