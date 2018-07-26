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
	index = curve->points.num;
	pos = _pos;
	value = _value;

	for (int i=0; i<curve->points.num; i++)
		if (pos < curve->points[i].pos){
			index = i;
			break;
		}
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
	curve->points.insert(p, index);

	//a->notify(a->MESSAGE_EDIT_CURVE);

	return nullptr;
}

void ActionCurveAddPoint::undo(Data* d)
{
	//Song *a = dynamic_cast<Song*>(d);

	curve->points.erase(index);
}

