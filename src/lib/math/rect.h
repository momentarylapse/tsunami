
#pragma once

#include "math.h"

class string;

class rect {
public:
	float x1, x2, y1, y2;
	rect() {};
	rect(float x1, float x2, float y1, float y2);
	string _cdecl str() const;
	float _cdecl width() const;
	float _cdecl height() const;
	float _cdecl area() const;
	float _cdecl mx() const;
	float _cdecl my() const;
	bool _cdecl inside(float x, float y) const;
	bool _cdecl covers(const rect &r) const;
	bool _cdecl overlaps(const rect &r) const;

	bool _cdecl operator==(const rect &r) const;
	bool _cdecl operator!=(const rect &r) const;
	rect _cdecl operator&&(const rect &r) const;
	rect _cdecl operator||(const rect &r) const;

	static const rect ID;
	static const rect EMPTY;
};
