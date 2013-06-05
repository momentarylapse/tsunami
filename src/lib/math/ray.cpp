/*
 * ray.cpp
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#include "math.h"



Ray::Ray(){}

Ray::Ray(const vector &a, const vector &b)
{
	u = b - a;
	v = b ^ a;
}

float Ray::dot(const Ray &r) const
{
	return (u * r.v) + (v * r.u);
}

bool Ray::intersect_plane(const plane &pl, vector &c) const
{
	float w = u * pl.n;
	if (w == 0)
		return false;
	c = ((v ^ pl.n) - u * pl.d) / w;
	return true;
}

