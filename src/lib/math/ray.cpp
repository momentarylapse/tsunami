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

float Ray::dot(const Ray &a, const Ray &b) {
	return vec3::dot(a.u, b.v) + vec3::dot(a.v, b.u);
}

base::optional<vec3> Ray::intersect_plane(const plane &pl) const {
	float w = vec3::dot(u, pl.n);
	if (w == 0)
		return base::None;
	return (vec3::cross(v, pl.n) - u * pl.d) / w;
}

