/*
 * ray.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#pragma once

#include "math.h"
#include "vec3.h"
#include "../base/optional.h"

struct vec3;
struct plane;


struct Ray {
	Ray();
	Ray(const vec3 &a, const vec3 &b);
	vec3 u, v;
	static float dot(const Ray &a, const Ray &b);
	base::optional<vec3> intersect_plane(const plane &pl) const;
};

