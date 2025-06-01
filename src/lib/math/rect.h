
#pragma once

#include "math.h"

struct string;
struct vec2;

struct rect {
	float x1, x2, y1, y2;
	rect() {};
	rect(float x1, float x2, float y1, float y2);
	rect(const vec2& p00, const vec2& p11);
	string str() const;
	float width() const;
	float height() const;
	float area() const;
	vec2 center() const;
	vec2 size() const;
	vec2 p00() const;
	vec2 p01() const;
	vec2 p10() const;
	vec2 p11() const;
	rect canonical() const;
	bool inside(const vec2 &p) const;
	bool covers(const rect &r) const;
	bool overlaps(const rect &r) const;
	rect grow(float d) const;

	bool operator==(const rect &r) const;
	bool operator!=(const rect &r) const;
	rect operator&&(const rect &r) const;
	rect operator||(const rect &r) const;

	static const rect ID;
	static const rect ID_SYM;
	static const rect EMPTY;
};
