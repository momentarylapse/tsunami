
#pragma once

#include "math.h"

class string;
class vec2;

class rect {
public:
	float x1, x2, y1, y2;
	rect() {};
	rect(float x1, float x2, float y1, float y2);
	string _cdecl str() const;
	float _cdecl width() const;
	float _cdecl height() const;
	float _cdecl area() const;
	vec2 _cdecl center() const;
	vec2 _cdecl size() const;
	bool _cdecl inside(const vec2 &p) const;
	bool _cdecl covers(const rect &r) const;
	bool _cdecl overlaps(const rect &r) const;

	bool _cdecl operator==(const rect &r) const;
	bool _cdecl operator!=(const rect &r) const;
	rect _cdecl operator&&(const rect &r) const;
	rect _cdecl operator||(const rect &r) const;

	static const rect ID;
	static const rect ID_SYM;
	static const rect EMPTY;
};
