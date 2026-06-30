#pragma once

#include "vec3.h"

struct Box {
	vec3 min, max;

	vec3 center() const;
	vec3 size() const;
	Box canonical() const;
	bool is_inside(const vec3& p) const;
	vec3 to_relative(const vec3& p) const;
	vec3 to_absolute(const vec3& p) const;
	vec3 clamp(const vec3& p) const;
	vec3 loop(const vec3& p) const;
	string str() const;

	bool operator==(const Box& o) const;
	bool operator!=(const Box& o) const;
	Box operator||(const Box& b) const;
	Box operator&&(const Box& b) const;
	Box operator+(const vec3& d) const;
	Box operator-(const vec3& d) const;
	Box operator*(float scale) const;
	void operator+=(const vec3& d);
	void operator-=(const vec3& d);
	void operator*=(float scale);

	static const Box EMPTY;
	static const Box ID;
	static const Box ID_SYM;
};

