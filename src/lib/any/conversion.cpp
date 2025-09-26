//
// Created by michi on 8/21/25.
//

#include "conversion.h"
#include <lib/math/vec2.h>
#include <lib/math/vec3.h>
#if __has_include(<lib/image/color.h>)
#include <lib/image/color.h>

color any_to_color(const Any& a) {
	const auto list = any_to_float_list<float>(a);
	// rgb-a  :P
	if (list.num >= 4)
		return color(list[3], list[0], list[1], list[2]);
	if (list.num >= 3)
		return color(1, list[0], list[1], list[2]);
	return Black;
}

Any color_to_any(const color& c) {
	Any r;
	r.add(Any(c.r));
	r.add(Any(c.g));
	r.add(Any(c.b));
	r.add(Any(c.a));
	return r;
}
#endif

vec3 any_to_vec3(const Any& a) {
	const auto list = any_to_float_list<float>(a);
	if (list.num >= 3)
		return {list[0], list[1], list[2]};
	return vec3::ZERO;
}

Any vec3_to_any(const vec3& v) {
	Any r;
	r.add(Any(v.x));
	r.add(Any(v.y));
	r.add(Any(v.z));
	return r;
}

vec2 any_to_vec2(const Any& a) {
	const auto list = any_to_float_list<float>(a);
	if (list.num >= 2)
		return {list[0], list[1]};
	return vec2::ZERO;
}

Any vec2_to_any(const vec2& v) {
	Any r;
	r.add(Any(v.x));
	r.add(Any(v.y));
	return r;
}

Any mat4_to_any(const mat4& m) {
	Any a = Any::EmptyList;
	for (int i=0; i<16; i++)
		a.list_set(i, Any(((float*)&m)[i]));
	return a;
}
