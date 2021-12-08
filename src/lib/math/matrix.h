
#pragma once


#include "math.h"

//#define _element(row,col)	e[row+col*4]

class matrix;
class vector;
class quaternion;
class plane;
//matrix MatrixMultiply2(const matrix &m2, const matrix &m1);


class matrix {
public:
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

	matrix(){};
	matrix(const float f[16]);
	matrix(const vector &a, const vector &b, const vector &c);
	matrix _cdecl operator + (const matrix &m) const;
	matrix _cdecl operator - (const matrix &m) const;
	matrix _cdecl operator * (const matrix &m) const;
	matrix _cdecl operator * (float f) const;
	friend matrix _cdecl operator * (float f, const matrix &m)
	{	return m*f;	}
	matrix _cdecl operator *= (const matrix &m);
	vector _cdecl operator * (const vector &v) const;
	float _cdecl determinant() const;
	vector _cdecl transform(const vector &v) const;
	vector _cdecl transform_normal(const vector &v) const;
	vector _cdecl untransform(const vector &v) const;
	vector _cdecl project(const vector &v) const;
	vector _cdecl unproject(const vector &v) const;
	string _cdecl str() const;
	matrix _cdecl inverse() const;
	matrix _cdecl transpose() const;

	// kaba
	void _cdecl imul(const matrix &m);
	matrix _cdecl mul(const matrix &m) const;
	vector _cdecl mul_v(const vector &v) const;

	static const matrix ID;

	// creation
	static matrix _cdecl translation(const vector &v);
	static matrix _cdecl rotation_x(float w);
	static matrix _cdecl rotation_y(float w);
	static matrix _cdecl rotation_z(float w);
	static matrix _cdecl rotation_v(const vector &ang);
	static matrix _cdecl rotation_q(const quaternion &q);
	static matrix _cdecl rotation(const vector &ang) { return rotation_v(ang); }
	static matrix _cdecl rotation(const quaternion &q) { return rotation_q(q); }
	static matrix _cdecl scale(float fx,float fy,float fz);
	static matrix _cdecl reflection(const plane &pl);
	static matrix _cdecl perspective(float fovy, float aspect, float z_near, float z_far, bool z_sym);
};


//inline vector *_matrix_get_translation_(const matrix &m)
//{	return (vector*)&m._03;	} // (_03, _13, _23) happens to be aligned the right way...

