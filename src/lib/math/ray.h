/*
 * ray.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#pragma once

#include "math.h"
#include "vec3.h"

class vec3;
class plane;


class Ray {
public:
	Ray();
	Ray(const vec3 &a, const vec3 &b);
	vec3 u, v;
	float _cdecl dot(const Ray &r) const;
	bool _cdecl intersect_plane(const plane &pl, vec3 &c) const;
};

