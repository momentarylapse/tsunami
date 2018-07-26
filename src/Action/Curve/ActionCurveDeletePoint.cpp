/*
 * ActionCurveDeletePoint.cpp
 *
 *  Created on: 06.10.2017
 *      Author: michi
 */

#include "ActionCurveDeletePoint.h"
#include "../../Data/Song.h"
#include "../../Data/Curve.h"


ActionCurveDeletePoint::ActionCurveDeletePoint(Curve *_curve, int _index)
{
	curve = _curve;
	index = _index;
	pos = 0;
	value = 0;
}

ActionCurveDeletePoint::~ActionCurveDeletePoint()
{
}

void* ActionCurveDeletePoint::execute(Data* d)
{
	//Song *a = dynamic_cast<Song*>(d);

	pos = curve->points[index].pos;
	value = curve->points[index].value;
	curve->points.erase(index);

	//a->notify(a->MESSAGE_EDIT_CURVE);

	return nullptr;
}

void ActionCurveDeletePoint::undo(Data* d)
{
	//Song *a = dynamic_cast<Song*>(d);

	Curve::Point p;
	p.pos = pos;
	p.value = value;
	curve->points.insert(p, index);

	//a->notify(a->MESSAGE_EDIT_CURVE);
}


