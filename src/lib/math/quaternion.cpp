#include "quaternion.h"
#include "vector.h"
#include "matrix.h"
#include "math.h"
#include <math.h>
#include "../os/msg.h"

//------------------------------------------------------------------------------------------------//
//                                          quaternions                                           //
//------------------------------------------------------------------------------------------------//


const quaternion quaternion::ID = quaternion(1, v_0);

quaternion::quaternion(const float w, const vector &v) {
	this->w = w;
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
}

/*quaternion::quaternion(const vector &v) {
	*this = rotation_v(v);
}*/

bool quaternion::operator == (const quaternion& q) const {
	return ((x==q.x) and (y==q.y) and (z==q.z) and (w==q.w));
}

bool quaternion::operator != (const quaternion& q) const {
	return !((x==q.x) and (y==q.y) and (z==q.z) and (w!=q.w));
}

void quaternion::operator += (const quaternion& q) {
	x += q.x;
	y += q.y;
	z += q.z;
	w += q.w;
}

void quaternion::operator -= (const quaternion& q) {
	x -= q.x;
	y -= q.y;
	z -= q.z;
	w -= q.w;
}

quaternion quaternion::operator + (const quaternion &q) const {
	quaternion r;
	r.x = q.x + x;
	r.y = q.y + y;
	r.z = q.z + z;
	r.w = q.w + w;
	return r;
}

quaternion quaternion::operator - (const quaternion &q) const {
	quaternion r;
	r.x = q.x - x;
	r.y = q.y - y;
	r.z = q.z - z;
	r.w = q.w - w;
	return r;
}

quaternion quaternion::operator * (float f) const {
	quaternion r;
	r.x = x * f;
	r.y = y * f;
	r.z = z * f;
	r.w = w * f;
	return r;
}

quaternion quaternion::operator * (const quaternion &q) const {
	quaternion r;
	r.w = w*q.w - x*q.x - y*q.y - z*q.z;
	r.x = w*q.x + x*q.w + y*q.z - z*q.y;
	r.y = w*q.y + y*q.w + z*q.x - x*q.z;
	r.z = w*q.z + z*q.w + x*q.y - y*q.x;
	return r;
}

vector quaternion::operator * (const vector &v) const {
	vector r = v * (w*w - x*x - y*y - z*z);
	vector *vv = (vector*)&x;
	r += 2 * w * vector::cross(*vv, v);
	r += 2 * vector::dot(*vv, v) * (*vv);
	return r;
}

string quaternion::str() const {
	return format("(%f, %f, %f, %f)", x, y, z, w);
}


// kaba
void quaternion::imul(const quaternion &q)
{	*this = (*this) * q;	}

quaternion quaternion::mul(const quaternion &q) const
{	return (*this) * q;	}

// rotation with an <angle w> and an <axis axis>
quaternion quaternion::rotation_a(const vector &axis, float w) {
	float w_half=w*0.5f;
	float s=sinf(w_half);
	return quaternion(cosf(w_half), axis * s);
}

// ZXY -> everything IN the game (world transformation)
quaternion quaternion::rotation_v(const vector &ang) {
	quaternion q;
	float wx_2=ang.x*0.5f;
	float wy_2=ang.y*0.5f;
	float wz_2=ang.z*0.5f;
	float cx=cosf(wx_2);
	float cy=cosf(wy_2);
	float cz=cosf(wz_2);
	float sx=sinf(wx_2);
	float sy=sinf(wy_2);
	float sz=sinf(wz_2);
	q.w=(cy*cx*cz) + (sy*sx*sz);
	q.x=(cy*sx*cz) + (sy*cx*sz);
	q.y=(sy*cx*cz) - (cy*sx*sz);
	q.z=(cy*cx*sz) - (sy*sx*cz);

	/*quaternion x,y,z;
	QuaternionRotationA(x,vector(1,0,0),ang.x);
	QuaternionRotationA(y,vector(0,1,0),ang.y);
	QuaternionRotationA(z,vector(0,0,1),ang.z);
	// y*x*z
	QuaternionMultiply(q,x,z);
	QuaternionMultiply(q,y,q);*/
	return q;
}

// create a quaternion from a (rotation-) matrix
quaternion quaternion::rotation_m(const matrix &m) {
	float tr = m._00 + m._11 + m._22;
	float w = acosf((tr - 1) / 2);
	
	if ((w < 0.00000001f) and (w > -0.00000001f))
		return ID;
		
	float s = 0.5f / sinf(w);
	vector n;
	n.x = (m._21 - m._12) * s;
	n.y = (m._02 - m._20) * s;
	n.z = (m._10 - m._01) * s;
	n.normalize();
	return rotation_a(n, w);
}

// invert a quaternion rotation
quaternion quaternion::inverse() const {
	return quaternion(w, vector(-x, -y, -z));
}

quaternion quaternion::bar() const {
	return quaternion(w, vector(-x, -y, -z));
}

// unite 2 rotations (first rotate by q1, then by q2: q = q2*q1)
void QuaternionMultiply(quaternion &q,const quaternion &q2,const quaternion &q1) {
	quaternion _q;
	_q.w = q2.w*q1.w - q2.x*q1.x - q2.y*q1.y - q2.z*q1.z;
	_q.x = q2.w*q1.x + q2.x*q1.w + q2.y*q1.z - q2.z*q1.y;
	_q.y = q2.w*q1.y + q2.y*q1.w + q2.z*q1.x - q2.x*q1.z;
	_q.z = q2.w*q1.z + q2.z*q1.w + q2.x*q1.y - q2.y*q1.x;
	q=_q;
}

// q = q1 + t*( q2 - q1)
quaternion quaternion::interpolate(const quaternion &q1,const quaternion &q2,float t) {
	//msg_todo("TestMe: QuaternionInterpolate(2q) for OpenGL");
	quaternion q=q1;

	t=1-t; // ....?

	// dot product = cos angle(q1,q2)
	float c = q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;
	float t2;
	bool flip=false;
	// flip, if q1 and q2 on opposite hemispheres
	if (c<0) {
		c=-c;
		flip=true;
	}
	// q1 and q2 "too equal"?
	if (c>0.9999f) {
		t2=1.0f-t;
	} else {
		float theta=acosf(c);
		float phi=theta;//+spin*pi; // spin for additional circulations...
		float s=sinf(theta);
		t2=sinf(theta-t*phi)/s;
		t=sinf(t*phi)/s;
	}
	if (flip)
		t=-t;

	q.x = t*q1.x + t2*q2.x;
	q.y = t*q1.y + t2*q2.y;
	q.z = t*q1.z + t2*q2.z;
	q.w = t*q1.w + t2*q2.w;
	return q;
}

quaternion quaternion::interpolate(const quaternion &q1,const quaternion &q2,const quaternion &q3,const quaternion &q4,float t) {
	/*#ifdef NIX_TYPES_BY_DIRECTX9
		D3DXQUATERNION A,B,C;
		D3DXQuaternionSquadSetup(&A,&B,&C,(D3DXQUATERNION*)&q1,(D3DXQUATERNION*)&q2,(D3DXQUATERNION*)&q3,(D3DXQUATERNION*)&q4);
		D3DXQuaternionSquad((D3DXQUATERNION*)&q,(D3DXQUATERNION*)&q2,&A,&B,&C,t);
	#else*/
	msg_todo("QuaternionInterpolate(4q)");
	return q2;
}

// convert a quaternion into 3 angles (ZXY)
vector quaternion::get_angles() const {
	vector ang;
	ang.x = asin(2*(w*x - z*y));
	ang.y = atan2(2*(w*y+x*z), 1-2*(y*y+x*x));
	ang.z = atan2(2*(w*z+x*y), 1-2*(z*z+x*x));
/*	// really bad!
	vector ang,v;
	matrix m,x,y;
	MatrixRotationQ(m,*this);
	v=m*vector(0,0,1000.0f);
	ang.y= atan2f(v.x,v.z);
	ang.x=-atan2f(v.y,sqrt(v.x*v.x+v.z*v.z));
	MatrixRotationX(x,-ang.x);
	MatrixRotationY(y,-ang.y);
	MatrixMultiply(m,y,m);
	MatrixMultiply(m,x,m);
	v=m*vector(1000.0f,0,0);
	ang.z=atan2f(v.y,v.x);*/
	return ang;
}

// scale the angle of the rotation
quaternion quaternion::scale_angle(float f) const {
	float w = get_angle();
	if (w==0)
		return *this;

	quaternion q;
	q.w=cosf(w*f/2);
	float factor=sinf(w*f/2)/sinf(w/2);
	q.x *= factor;
	q.y *= factor;
	q.z *= factor;
	return q;
}

// quaternion correction
void quaternion::normalize() {
	float l = sqrt(x*x + y*y + z*z + w*w);
	l = 1.0f / l;
	w *= l;
	x *= l;
	y *= l;
	z *= l;
}

// the axis of our quaternion rotation
vector quaternion::get_axis() const {
	return vector(x, y, z).normalized();
}

// angle value of the quaternion
float quaternion::get_angle() const {
	return acos(w)*2;
}

quaternion quaternion::drag(const vector &up, const vector &dang, bool reset_z) {
	quaternion T, TT, q;
	bool is_not_z = (up.x != 0) or (up.y != 0) or (up.z < 0);
	if (is_not_z) {
		vector ax = vector::cross(vector::EZ, up);
		ax.normalize();
		vector up2 = up;
		up2.normalize();
		T = rotation_a(ax, acos(up2.z));
		TT = T.bar();
		q = T * q * TT;
	}

	vector ang = q.get_angles();
	ang.x = clamp(ang.x + dang.x, -pi/2+0.01f, pi/2-0.01f);
	ang.y += dang.y;
	ang.z += dang.z;
	if (reset_z)
		ang.z = 0;
	q = rotation_v(ang);

	if (is_not_z)
		q = TT * q * T;
	return q;
}

bool inf_q(const quaternion &q) {
	return (inf_f(q.x) or inf_f(q.y) or inf_f(q.z) or inf_f(q.z));
}
