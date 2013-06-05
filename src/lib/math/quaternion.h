
#ifndef _MATH_QUATERNION_INCLUDED_
#define _MATH_QUATERNION_INCLUDED_

class vector;
class matrix;

struct quaternion
{
public:
	float x, y, z, w;
	quaternion(){};
	quaternion(const float w,const vector &v);
	bool operator == (const quaternion& q) const;
	bool operator != (const quaternion& q) const;
	quaternion& operator += (const quaternion& q);
	quaternion& operator -= (const quaternion& q);
	quaternion operator + (const quaternion &q) const;
	quaternion operator - (const quaternion &q) const;
	quaternion operator * (float f) const;
	quaternion operator * (const quaternion &q) const;
	friend quaternion operator * (float f,const quaternion &q)
	{	return q*f;	}
	string str() const;

	void normalize();
	void inverse();
	vector get_axis() const;
	float get_angle() const;
	vector get_angles() const;

	// kaba
	void imul(const quaternion &q);
	quaternion mul(const quaternion &q) const;
};
// qaternions
void _cdecl QuaternionRotationA(quaternion &q, const vector &axis, float w);
void _cdecl QuaternionRotationV(quaternion &q, const vector &ang);
//void QuaternionRotationView(quaternion &q, const vector &ang);
void _cdecl QuaternionRotationM(quaternion &q, const matrix &m);
void _cdecl QuaternionInterpolate(quaternion &q, const quaternion &q1, const quaternion &q2, float t);
void _cdecl QuaternionInterpolate(quaternion &q, const quaternion &q1, const quaternion &q2, const quaternion &q3, const quaternion &q4, float t);
void _cdecl QuaternionScale(quaternion &q, float f);

const quaternion q_id = quaternion(1, v_0);

#endif
