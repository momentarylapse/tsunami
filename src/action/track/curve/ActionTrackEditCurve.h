/*
 * ActionTrackEditCurve.h
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>
#include "../../../data/Curve.h"

namespace tsunami {

struct Curve;
enum class CurveType;

class ActionTrackEditCurve : public history::Action {
public:
	ActionTrackEditCurve(Track *t, shared<Curve> curve, const string &name, float min, float max, CurveType type);

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	Track *track;
	shared<Curve> curve;
	string name;
	float min, max;
	CurveType type;
};

}
