/*
 * ray.cpp
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#include "ray.h"
#include "plane.h"



Ray::Ray(){}

Ray::Ray(const vector &a, const vector &b) {
	u = b - a;
	v = vector::cross(b, a);
}

float Ray::dot(const Ray &r) const {
	return vector::dot(u, r.v) + vector::dot(v, r.u);
}

bool Ray::intersect_plane(const plane &pl, vector &c) const {
	float w = vector::dot(u, pl.n);
	if (w == 0)
		return false;
	c = (vector::cross(v, pl.n) - u * pl.d) / w;
	return true;
}

