
#pragma once


#include "math.h"

//#define _element(row,col)	e[row+col*4]

struct mat4;
struct vec3;
struct quaternion;
struct plane;
//matrix MatrixMultiply2(const matrix &m2, const matrix &m1);


struct mat4 {
	union {
		struct {
			// "OpenGL" column major order
			float _00,_10,_20,_30;
			float _01,_11,_21,_31;
			float _02,_12,_22,_32;
			float _03,_13,_23,_33;
		};
		float __e[4][4];
		float e[16];
	};

	mat4(){};
	mat4(const float f[16]);
	mat4(const vec3 &a, const vec3 &b, const vec3 &c);
	mat4 _cdecl operator + (const mat4 &m) const;
	mat4 _cdecl operator - (const mat4 &m) const;
	mat4 _cdecl operator * (const mat4 &m) const;
	mat4 _cdecl operator * (float f) const;
	friend mat4 _cdecl operator * (float f, const mat4 &m)
	{	return m*f;	}
	mat4 _cdecl operator *= (const mat4 &m);
	vec3 _cdecl operator * (const vec3 &v) const;
	float _cdecl determinant() const;
	vec3 _cdecl transform(const vec3 &v) const;
	vec3 _cdecl transform_normal(const vec3 &v) const;
	vec3 _cdecl untransform(const vec3 &v) const;
	vec3 _cdecl project(const vec3 &v) const;
	vec3 _cdecl unproject(const vec3 &v) const;
	string _cdecl str() const;
	mat4 _cdecl inverse() const;
	mat4 _cdecl transpose() const;

	static const mat4 ID;
	static const mat4 ZERO;

	// creation
	static mat4 _cdecl translation(const vec3 &v);
	static mat4 _cdecl rotation_x(float w);
	static mat4 _cdecl rotation_y(float w);
	static mat4 _cdecl rotation_z(float w);
	static mat4 _cdecl rotation(const vec3 &ang);
	static mat4 _cdecl rotation(const quaternion &q);
	static mat4 _cdecl scale(float fx, float fy, float fz);
	static mat4 _cdecl scale(const vec3 &v);
	static mat4 _cdecl reflection(const plane &pl);
	static mat4 _cdecl perspective(float fovy, float aspect, float z_near, float z_far, bool z_sym);
};


//inline vector *_matrix_get_translation_(const matrix &m)
//{	return (vector*)&m._03;	} // (_03, _13, _23) happens to be aligned the right way...

