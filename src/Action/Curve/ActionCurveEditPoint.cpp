/*
 * ActionCurveEditPoint.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionCurveEditPoint.h"
#include "../../Data/Song.h"
#include "../../Data/Curve.h"

ActionCurveEditPoint::ActionCurveEditPoint(Curve *_curve, int _index, int _pos, float _value)
{
	curve = _curve;
	index = _index;
	new_value.pos = _pos;
	new_value.value = _value;
}

ActionCurveEditPoint::~ActionCurveEditPoint()
{
}

void* ActionCurveEditPoint::execute(Data* d)
{
	//Song *a = dynamic_cast<Song*>(d);

	old_value = curve->points[index];
	curve->points[index] = new_value;

	return nullptr;
}

void ActionCurveEditPoint::undo(Data* d)
{
	//Song *a = dynamic_cast<Song*>(d);

	curve->points[index] = old_value;
}

bool ActionCurveEditPoint::mergable(Action *a)
{
	ActionCurveEditPoint *e = dynamic_cast<ActionCurveEditPoint*>(a);
	if (!e)
		return false;
	return (curve == e->curve) and (index == e->index);

}

