
#pragma once


#include "math.h"

class matrix;
class matrix3;

class vec2 {
public:
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

	static const vec2 ZERO, EX, EY;
};

class vector {
public:
	float x, y, z;
	vector(){};
	vector(float x, float y, float z);
	// assignment operators
	void _cdecl operator += (const vector& v);
	void _cdecl operator -= (const vector& v);
	void _cdecl operator *= (float f);
	void _cdecl operator /= (float f);
	// unitary operator(s)
	vector _cdecl operator - () const;
	// binary operators
	vector _cdecl operator + (const vector &v) const;
	vector _cdecl operator - (const vector &v) const;
	vector _cdecl operator * (float f) const;
	vector _cdecl operator / (float f) const;
	friend vector _cdecl operator * (float f,const vector &v)
	{	return v*f;	}
	bool _cdecl operator == (const vector &v) const;
	bool _cdecl operator != (const vector &v) const;
	float _cdecl operator * (const vector &v) const;
	vector operator ^ (const vector &v) const;
	string _cdecl str() const;

	float _cdecl length() const;
	float _cdecl length_sqr() const;
	float _cdecl length_fuzzy() const;
	void _cdecl normalize();
	vector _cdecl normalized() const;
	vector _cdecl ang2dir() const;
	vector _cdecl dir2ang() const;
	vector _cdecl dir2ang2(const vector &up) const;
	vector _cdecl ortho() const;
	int _cdecl important_plane() const;
//	vector _cdecl rotate(const vector &ang) const;
//	vector _cdecl transform(const matrix &m) const;
//	vector _cdecl transform_normal(const matrix &m) const;
//	vector _cdecl untransform(const matrix &m) const;
//	vector _cdecl transform3(const matrix3 &m) const;
	void _cdecl _min(const vector &test_partner);
	void _cdecl _max(const vector &test_partner);
	bool _cdecl between(const vector &a, const vector &b) const;
	float _cdecl factor_between(const vector &a, const vector &b) const;
	bool _cdecl bounding_cube(const vector &a, float r) const;

	static const vector ZERO, EX, EY, EZ;
	
	static float _cdecl dot(const vector &v1, const vector &v2);
	static vector _cdecl cross(const vector &v1, const vector &v2);
};
using vec3 = vector;
//typedef vec3 vector;

// vectors
//vector _cdecl VecAngAdd(const vector &ang1, const vector &ang2);
//vector _cdecl VecAngInterpolate(const vector &ang1, const vector &ang2, float t);
float _cdecl VecLineDistance(const vector &p, const vector &l1, const vector &l2);
vector _cdecl VecLineNearestPoint(const vector &p, const vector &l1, const vector &l2);


const vector v_0 = vector(0, 0, 0);


float _vec_length_(const vector &v);
float _vec_length_fuzzy_(const vector &v);
void _vec_normalize_(vector &v);
bool _vec_between_(const vector &v,const vector &a,const vector &b);
float _vec_factor_between_(const vector &v,const vector &a,const vector &b);

bool inf_v(const vector &v);




class vec4 {
public:
	float x, y, z, w;
	vec4(){};
	vec4(float x, float y, float z, float w);
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

