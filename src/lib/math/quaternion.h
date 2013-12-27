
#ifndef _MATH_QUATERNION_INCLUDED_
#define _MATH_QUATERNION_INCLUDED_

class vector;
class matrix;

class quaternion
{
public:
	float x, y, z, w;
	quaternion(){};
	quaternion(const float w, const vector &v);
	quaternion(const vector &v);
	bool _cdecl operator == (const quaternion& q) const;
	bool _cdecl operator != (const quaternion& q) const;
	quaternion& _cdecl operator += (const quaternion& q);
	quaternion& _cdecl operator -= (const quaternion& q);
	quaternion _cdecl operator + (const quaternion &q) const;
	quaternion _cdecl operator - (const quaternion &q) const;
	quaternion _cdecl operator * (float f) const;
	quaternion _cdecl operator * (const quaternion &q) const;
	vector _cdecl operator * (const vector &v) const;
	friend quaternion _cdecl operator * (float f,const quaternion &q)
	{	return q*f;	}
	string _cdecl str() const;

	void _cdecl normalize();
	void _cdecl invert();
	quaternion _cdecl bar() const;
	vector _cdecl get_axis() const;
	float _cdecl get_angle() const;
	vector get_angles() const;

	// kaba
	void _cdecl imul(const quaternion &q);
	quaternion _cdecl mul(const quaternion &q) const;
};
// qaternions
void _cdecl QuaternionRotationA(quaternion &q, const vector &axis, float w);
void _cdecl QuaternionRotationV(quaternion &q, const vector &ang);
//void QuaternionRotationView(quaternion &q, const vector &ang);
void _cdecl QuaternionRotationM(quaternion &q, const matrix &m);
void _cdecl QuaternionInterpolate(quaternion &q, const quaternion &q1, const quaternion &q2, float t);
void _cdecl QuaternionInterpolate(quaternion &q, const quaternion &q1, const quaternion &q2, const quaternion &q3, const quaternion &q4, float t);
void _cdecl QuaternionScale(quaternion &q, float f);
void _cdecl QuaternionDrag(quaternion &q, const vector &up, const vector &dang, bool reset_z);

const quaternion q_id = quaternion(1, v_0);

#endif
