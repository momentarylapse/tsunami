
#pragma once


#include "math.h"

struct mat4;
struct mat3;
struct vec2;

struct vec3 {
	float x, y, z;
	vec3(){};
	vec3(float x, float y, float z);
	vec3(const vec2 &xy, float z);

	vec2 &xy();

	// assignment operators
	void _cdecl operator += (const vec3& v);
	void _cdecl operator -= (const vec3& v);
	void _cdecl operator *= (float f);
	void _cdecl operator /= (float f);
	// unitary operator(s)
	vec3 _cdecl operator - () const;
	// binary operators
	vec3 _cdecl operator + (const vec3 &v) const;
	vec3 _cdecl operator - (const vec3 &v) const;
	vec3 _cdecl operator * (float f) const;
	vec3 _cdecl operator / (float f) const;
	friend vec3 _cdecl operator * (float f,const vec3 &v)
	{	return v*f;	}
	bool _cdecl operator == (const vec3 &v) const;
	bool _cdecl operator != (const vec3 &v) const;
	float _cdecl operator * (const vec3 &v) const;
	vec3 operator ^ (const vec3 &v) const;
	string _cdecl str() const;

	float _cdecl length() const;
	float _cdecl length_sqr() const;
	float _cdecl length_fuzzy() const;
	void _cdecl normalize();
	vec3 _cdecl normalized() const;
	vec3 _cdecl ang2dir() const;
	vec3 _cdecl dir2ang() const;
	vec3 _cdecl dir2ang2(const vec3 &up) const;
	vec3 _cdecl ortho() const;
	vec2& xy() const;
	int _cdecl important_plane() const;
//	vector _cdecl rotate(const vector &ang) const;
//	vector _cdecl transform(const matrix &m) const;
//	vector _cdecl transform_normal(const matrix &m) const;
//	vector _cdecl untransform(const matrix &m) const;
//	vector _cdecl transform3(const matrix3 &m) const;
	void _cdecl _min(const vec3 &test_partner);
	void _cdecl _max(const vec3 &test_partner);
	bool _cdecl between(const vec3 &a, const vec3 &b) const;
	float _cdecl factor_between(const vec3 &a, const vec3 &b) const;
	bool _cdecl bounding_cube(const vec3 &a, float r) const;

	static const vec3 ZERO, EX, EY, EZ;
	
	static float _cdecl dot(const vec3 &v1, const vec3 &v2);
	static vec3 _cdecl cross(const vec3 &v1, const vec3 &v2);
};
using vector = vec3;

// vectors
//vector _cdecl VecAngAdd(const vector &ang1, const vector &ang2);
//vector _cdecl VecAngInterpolate(const vector &ang1, const vector &ang2, float t);
float _cdecl VecLineDistance(const vec3 &p, const vec3 &l1, const vec3 &l2);
vec3 _cdecl VecLineNearestPoint(const vec3 &p, const vec3 &l1, const vec3 &l2);


const vec3 v_0 = vec3(0, 0, 0);


float _vec_length_(const vec3 &v);
float _vec_length_fuzzy_(const vec3 &v);
void _vec_normalize_(vec3 &v);
bool _vec_between_(const vec3 &v,const vec3 &a,const vec3 &b);
float _vec_factor_between_(const vec3 &v,const vec3 &a,const vec3 &b);

bool inf_v(const vec3 &v);


struct ivec3 {
	int i,j,k;

	string str() const;
};


