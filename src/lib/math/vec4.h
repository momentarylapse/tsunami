/*
 * vec4.h
 *
 *  Created on: Jul 22, 2021
 *      Author: michi
 */


#pragma once


#include "math.h"



class vec4 {
public:
	float x, y, z, w;
	vec4() {}
	vec4(float x, float y, float z, float w);
	vec4(const vec3& v, float w);
	// assignment operators
	void _cdecl operator += (const vec4& v);
	void _cdecl operator -= (const vec4& v);
	void _cdecl operator *= (float f);
	void _cdecl operator /= (float f);
	// unitary operator(s)
	vec4 _cdecl operator - () const;
	// binary operators
	vec4 _cdecl operator + (const vec4 &v) const;
	vec4 _cdecl operator - (const vec4 &v) const;
	vec4 _cdecl operator * (float f) const;
	vec4 _cdecl operator / (float f) const;
	friend vec4 _cdecl operator * (float f, const vec4 &v)
	{	return v*f;	}
	bool _cdecl operator == (const vec4 &v) const;
	bool _cdecl operator != (const vec4 &v) const;
	float _cdecl operator * (const vec4 &v) const;
	string _cdecl str() const;

	float _cdecl length() const;
	float _cdecl length_sqr() const;
	float _cdecl length_fuzzy() const;
	void _cdecl normalize();
	vec4 _cdecl normalized() const;

	int argmin() const;
	int argmax() const;
	float sum() const;
	float &operator[](int index);
	float operator[](int index) const;

	static const vec4 ZERO, EX, EY, EZ, EW;
};


class ivec4 {
public:
	int i,j,k,l;

	int find(int x) const;
	int &operator[](int index);
	int operator[](int index) const;
};

