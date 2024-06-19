/*
 * ActionTrackEditCurve.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#pragma once

#include "../../Action.h"
#include "../../../data/Curve.h"

namespace tsunami {

class Curve;
enum class CurveType;

class ActionTrackEditCurve : public Action {
public:
	ActionTrackEditCurve(Track *t, shared<Curve> curve, const string &name, float min, float max, CurveType type);

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Track *track;
	shared<Curve> curve;
	string name;
	float min, max;
	CurveType type;
};

}
