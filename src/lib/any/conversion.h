//
// Created by michi on 8/21/25.
//

#pragma once

#include "any.h"

struct vec3;
struct vec2;
struct mat4;
struct color;

template <typename T>
Array<T> any_to_float_list(const Any& a) {
	Array<T> r;
	if (a.is_list())
		for (int i=0; i<a.as_list().num; i++)
			r.add((T)(a.as_list()[i]).to_f64());
	return r;
}

template <typename T>
Any float_list_to_any(const Array<T>& a) {
	Any r = Any::EmptyList;
	for (T x: a)
		r.add(Any(x));
	return r;
}

color any_to_color(const Any& a);
Any color_to_any(const color& c);
vec3 any_to_vec3(const Any& a);
Any vec3_to_any(const vec3& v);
vec2 any_to_vec2(const Any& a);
Any vec2_to_any(const vec2& v);
Any mat4_to_any(const mat4& m);