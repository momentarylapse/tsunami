
#pragma once


#include "math.h"

class vector;
class matrix;
class string;

class quaternion {
public:
	float x, y, z, w;
	quaternion(){};
	quaternion(const float w, const vector &v);
	//quaternion(const vector &v);
	bool _cdecl operator == (const quaternion& q) const;
	bool _cdecl operator != (const quaternion& q) const;
	void _cdecl operator += (const quaternion& q);
	void _cdecl operator -= (const quaternion& q);
	quaternion _cdecl operator + (const quaternion &q) const;
	quaternion _cdecl operator - (const quaternion &q) const;
	quaternion _cdecl operator * (float f) const;
	quaternion _cdecl operator * (const quaternion &q) const;
	vector _cdecl operator * (const vector &v) const;
	friend quaternion _cdecl operator * (float f,const quaternion &q)
	{	return q*f;	}
	string _cdecl str() const;

	void _cdecl normalize();
	quaternion _cdecl inverse() const;
	quaternion _cdecl bar() const;
	vector _cdecl get_axis() const;
	float _cdecl get_angle() const;
	vector get_angles() const;
	quaternion _cdecl scale_angle(float f) const;

	// kaba
	void _cdecl imul(const quaternion &q);
	quaternion _cdecl mul(const quaternion &q) const;

	static const quaternion ID;
	
	
	// creation
	static quaternion _cdecl rotation_a(const vector &axis, float w);
	static quaternion _cdecl rotation_v(const vector &ang);
	static quaternion _cdecl rotation_m(const matrix &m);
	static quaternion _cdecl rotation(const vector &axis, float w) { return rotation_a(axis, w); }
	static quaternion _cdecl rotation(const vector &ang) { return rotation_v(ang); }
	static quaternion _cdecl rotation(const matrix &m) { return rotation_m(m); }
	static quaternion _cdecl interpolate(const quaternion &q1, const quaternion &q2, float t);
	static quaternion _cdecl interpolate(const quaternion &q1, const quaternion &q2, const quaternion &q3, const quaternion &q4, float t);
	static quaternion _cdecl drag(const vector &up, const vector &dang, bool reset_z);
};



bool inf_q(const quaternion &q);
