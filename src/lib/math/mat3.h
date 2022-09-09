
#pragma once


#include "math.h"

class vec3;

class mat3 {
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

	mat3(){};
	mat3(const float f[9]);
	mat3 _cdecl operator + (const mat3 &m) const;
	mat3 _cdecl operator - (const mat3 &m) const;
	mat3 _cdecl operator * (float f) const;
	mat3 _cdecl operator *= (float f);
	friend mat3 _cdecl operator * (float f, const mat3 &m)
	{	return m * f;	}
	mat3 _cdecl operator / (float f) const;
	mat3 _cdecl operator /= (float f);
	mat3 _cdecl operator * (const mat3 &m) const;
	void _cdecl operator *= (const mat3 &m);
	vec3 _cdecl operator * (const vec3 &v) const;
	/*friend vector operator * (const vector &v, const matrix3 &m)
	{	return m*v;	}*/
	string _cdecl str() const;


	static const mat3 ID;
	static const mat3 ZERO;
	
	
	mat3 _cdecl inverse() const;
	mat3 _cdecl transpose() const;
	static mat3 _cdecl rotation(const vec3 &ang);
	static mat3 _cdecl rotation(const quaternion &q);
	static mat3 scale(float x, float y, float z);
	static mat3 scale(const vec3& v);
};


