/*
 * ActionTrackDeleteCurve.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionTrackDeleteCurve.h"
#include "../../../data/Track.h"
#include "../../../data/Curve.h"

ActionTrackDeleteCurve::ActionTrackDeleteCurve(Track *t, int _index) {
	track = t;
	index = _index;
}

void* ActionTrackDeleteCurve::execute(Data* d) {
	curve = track->curves[index];
	curve->fake_death();
	track->curves.erase(index);

	track->out_delete_curve.notify();

	return nullptr;
}

void ActionTrackDeleteCurve::undo(Data* d) {
	track->curves.insert(curve, index);
	track->out_add_curve.notify();
}

