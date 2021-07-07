
#pragma once

#include "math.h"

class string;

class complex {
public:
	float x, y;
	complex(){};
	complex(float x, float y);
	// assignment operators
	void operator += (const complex& v);
	void operator -= (const complex& v);
	void operator *= (float f);
	void operator /= (float f);
	void operator *= (const complex &v);
	void operator /= (const complex &v);
	// unitary operator(s)
	complex operator - ();
	// binary operators
	complex operator + (const complex &v) const;
	complex operator - (const complex &v) const;
	complex operator * (float f) const;
	complex operator / (float f) const;
	complex operator * (const complex &v) const;
	complex operator / (const complex &v) const;
	friend complex operator * (float f, const complex &v)
	{	return v*f;	}
	bool operator == (const complex &v) const;
	bool operator != (const complex &v) const;
	float  abs() const;
	float _cdecl abs_sqr() const;
	complex _cdecl bar() const;
	string _cdecl str() const;

	static const complex ZERO;
	static const complex ONE;
	static const complex I;
};


