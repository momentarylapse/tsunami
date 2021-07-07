/*
 * ray.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#pragma once

#include "math.h"
#include "vector.h"

class vector;
class plane;


class Ray {
public:
	Ray();
	Ray(const vector &a, const vector &b);
	vector u, v;
	float _cdecl dot(const Ray &r) const;
	bool _cdecl intersect_plane(const plane &pl, vector &c) const;
};

