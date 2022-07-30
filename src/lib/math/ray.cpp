/*
 * ray.cpp
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#include "ray.h"
#include "plane.h"



Ray::Ray(){}

Ray::Ray(const vec3 &a, const vec3 &b) {
	u = b - a;
	v = vec3::cross(b, a);
}

float Ray::dot(const Ray &r) const {
	return vec3::dot(u, r.v) + vec3::dot(v, r.u);
}

bool Ray::intersect_plane(const plane &pl, vec3 &c) const {
	float w = vec3::dot(u, pl.n);
	if (w == 0)
		return false;
	c = (vec3::cross(v, pl.n) - u * pl.d) / w;
	return true;
}

