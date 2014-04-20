/*
 * Curve.cpp
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#include "Curve.h"

Curve::Curve() :
	Observable("Curve")
{
	min = 0;
	max = 1;
	type = TYPE_LINEAR;
}

Curve::~Curve()
{
}

void Curve::add(int pos, float value)
{
	Point p;
	p.pos = pos;
	p.value = value;
	for (int i=0; i<points.num; i++)
		if (pos < points[i].pos){
			points.insert(p, i);
			return;
		}
	points.add(p);
}

float Curve::get(int pos)
{
	if (points.num == 0)
		return min;
	if (pos < points[0].pos)
		return points[0].value;
	for (int i=1; i<points.num; i++)
		if (pos < points[i].pos){
			float dv = points[i].value - points[i-1].value;
			float dp = points[i].pos - points[i-1].pos;
			return points[i-1].value + dv * (pos - points[i-1].pos) / dp;
		}
	return points.back().value;
}

