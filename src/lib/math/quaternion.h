
#pragma once


#include "math.h"

class vec3;
class mat4;
class string;

class quaternion {
public:
	float x, y, z, w;
	quaternion(){};
	quaternion(const float w, const vec3 &v);
	//quaternion(const vector &v);
	bool _cdecl operator == (const quaternion& q) const;
	bool _cdecl operator != (const quaternion& q) const;
	void _cdecl operator += (const quaternion& q);
	void _cdecl operator -= (const quaternion& q);
	quaternion _cdecl operator + (const quaternion &q) const;
	quaternion _cdecl operator - (const quaternion &q) const;
	quaternion _cdecl operator * (float f) const;
	quaternion _cdecl operator * (const quaternion &q) const;
	vec3 _cdecl operator * (const vec3 &v) const;
	friend quaternion _cdecl operator * (float f,const quaternion &q)
	{	return q*f;	}
	string _cdecl str() const;

	void _cdecl normalize();
	quaternion _cdecl inverse() const;
	quaternion _cdecl bar() const;
	vec3 _cdecl get_axis() const;
	float _cdecl get_angle() const;
	vec3 get_angles() const;
	quaternion _cdecl scale_angle(float f) const;

	// kaba
	void _cdecl imul(const quaternion &q);
	quaternion _cdecl mul(const quaternion &q) const;

	static const quaternion ID;
	
	
	// creation
	static quaternion _cdecl rotation_a(const vec3 &axis, float w);
	static quaternion _cdecl rotation_v(const vec3 &ang);
	static quaternion _cdecl rotation_m(const mat4 &m);
	static quaternion _cdecl rotation(const vec3 &axis, float w) { return rotation_a(axis, w); }
	static quaternion _cdecl rotation(const vec3 &ang) { return rotation_v(ang); }
	static quaternion _cdecl rotation(const mat4 &m) { return rotation_m(m); }
	static quaternion _cdecl interpolate(const quaternion &q1, const quaternion &q2, float t);
	static quaternion _cdecl interpolate(const quaternion &q1, const quaternion &q2, const quaternion &q3, const quaternion &q4, float t);
	static quaternion _cdecl drag(const vec3 &up, const vec3 &dang, bool reset_z);
};



bool inf_q(const quaternion &q);
