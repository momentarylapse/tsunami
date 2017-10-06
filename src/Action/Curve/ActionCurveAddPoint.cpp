/*
 * ActionCurveAddPoint.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionCurveAddPoint.h"
#include "../../Data/Song.h"
#include "../../Data/Curve.h"

ActionCurveAddPoint::ActionCurveAddPoint(Curve *_curve, int _pos, float _value)
{
	curve = _curve;
	index = 0;
	pos = _pos;
	value = _value;
}

ActionCurveAddPoint::~ActionCurveAddPoint()
{
}

void* ActionCurveAddPoint::execute(Data* d)
{
	//Song *a = dynamic_cast<Song*>(d);

	Curve::Point p;
	p.pos = pos;
	p.value = value;
	curve->points.add(p);
	index = curve->points.num - 1;

	//a->notify(a->MESSAGE_EDIT_CURVE);

	return NULL;
}

void ActionCurveAddPoint::undo(Data* d)
{
	//Song *a = dynamic_cast<Song*>(d);

	curve->points.erase(index);
}

