
#pragma once


#include "math.h"

class vector;

class matrix3 {
public:
	union {
		struct {
			// the squared form of this block is "transposed"!
			float _00,_10,_20;
			float _01,_11,_21;
			float _02,_12,_22;
		};
		float __e[3][3];
#define _e3(i,j)		__e[j][i]
		float e[9];
	};

	matrix3(){};
	matrix3(const float f[9]);
	matrix3 _cdecl operator + (const matrix3 &m) const;
	matrix3 _cdecl operator - (const matrix3 &m) const;
	matrix3 _cdecl operator * (float f) const;
	matrix3 _cdecl operator *= (float f);
	friend matrix3 _cdecl operator * (float f, const matrix3 &m)
	{	return m * f;	}
	matrix3 _cdecl operator / (float f) const;
	matrix3 _cdecl operator /= (float f);
	matrix3 _cdecl operator * (const matrix3 &m) const;
	void _cdecl operator *= (const matrix3 &m);
	vector _cdecl operator * (const vector &v) const;
	/*friend vector operator * (const vector &v, const matrix3 &m)
	{	return m*v;	}*/
	string _cdecl str() const;

	// kaba
	void _cdecl imul(const matrix3 &m);
	matrix3 _cdecl mul(const matrix3 &m) const;
	vector _cdecl mul_v(const vector &v) const;


	static const matrix3 ID;
	
	
	matrix3 _cdecl inverse() const;
	matrix3 _cdecl transpose() const;
	static matrix3 _cdecl rotation(const vector &ang);
	static matrix3 _cdecl rotation_q(const quaternion &q);
};


