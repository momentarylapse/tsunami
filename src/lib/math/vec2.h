/*
 * vec2.h
 *
 *  Created on: Jul 22, 2021
 *      Author: michi
 */


#pragma once


#include "math.h"

struct vec2 {
	float x, y;
	vec2(){};
	vec2(float x, float y);
	// assignment operators
	void _cdecl operator += (const vec2& v);
	void _cdecl operator -= (const vec2& v);
	void _cdecl operator *= (float f);
	void _cdecl operator /= (float f);
	// unitary operator(s)
	vec2 _cdecl operator - () const;
	// binary operators
	vec2 _cdecl operator + (const vec2 &v) const;
	vec2 _cdecl operator - (const vec2 &v) const;
	vec2 _cdecl operator * (float f) const;
	vec2 _cdecl operator / (float f) const;
	friend vec2 _cdecl operator * (float f, const vec2 &v)
	{	return v*f;	}
	bool _cdecl operator == (const vec2 &v) const;
	bool _cdecl operator != (const vec2 &v) const;
	float _cdecl operator * (const vec2 &v) const;
	string _cdecl str() const;

	float _cdecl length() const;
	float _cdecl length_sqr() const;
	float _cdecl length_fuzzy() const;
	void _cdecl normalize();
	vec2 _cdecl normalized() const;
	vec2 _cdecl ortho() const;
	static vec2 min(const vec2& a, const vec2& b);
	static vec2 max(const vec2& a, const vec2& b);

	static float cross(const vec2 &a, const vec2 &b);
	static float dot(const vec2 &a, const vec2 &b);

	// P = A + f*( B - A ) + g*( C - A )
	static vec2 bary_centric(const vec2 &P,const vec2 &A,const vec2 &B, const vec2 &C);

	static const vec2 ZERO, EX, EY;
};


struct ivec2 {
	int i,j;

	string str() const;
};

