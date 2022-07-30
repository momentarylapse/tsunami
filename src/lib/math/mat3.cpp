#include "mat3.h"
#include "vec3.h"
#include "quaternion.h"
#include "../os/msg.h"

//------------------------------------------------------------------------------------------------//
//                                          3x3-matrices                                          //
//------------------------------------------------------------------------------------------------//


const float f_m3_id[9] = { 1,0,0 , 0,1,0 , 0,0,1 };
const float f_m3_zero[9] = { 0,0,0 , 0,0,0 , 0,0,0 };
const mat3 mat3::ID = mat3(f_m3_id);
const mat3 mat3::ZERO = mat3(f_m3_zero);

mat3::mat3(const float f[9]) {
	for (int i=0;i<9;i++)
		e[i]=f[i];
}


mat3 mat3::operator + (const mat3 &m) const {
	mat3 r;
	for (int i=0;i<9;i++)
		r.e[i]=e[i]+m.e[i];
	return r;
}

mat3 mat3::operator - (const mat3 &m) const {
	mat3 r;
	for (int i=0;i<9;i++)
		r.e[i]=e[i]-m.e[i];
	return r;
}

mat3 mat3::operator * (float f) const {
	mat3 r = *this;
	r *= f;
	return r;
}

mat3 mat3::operator *= (float f) {
	for (int i=0;i<9;i++)
		e[i] *= f;
	return *this;
}

mat3 mat3::operator / (float f) const {
	mat3 r = *this;
	r /= f;
	return r;
}

mat3 mat3::operator /= (float f) {
	for (int i=0;i<9;i++)
		e[i] /= f;
	return *this;
}

mat3 mat3::operator * (const mat3 &m) const {
	mat3 r;
	r._00 = _00*m._00 + _01*m._10 + _02*m._20;
	r._01 = _00*m._01 + _01*m._11 + _02*m._21;
	r._02 = _00*m._02 + _01*m._12 + _02*m._22;
	r._10 = _10*m._00 + _11*m._10 + _12*m._20;
	r._11 = _10*m._01 + _11*m._11 + _12*m._21;
	r._12 = _10*m._02 + _11*m._12 + _12*m._22;
	r._20 = _20*m._00 + _21*m._10 + _22*m._20;
	r._21 = _20*m._01 + _21*m._11 + _22*m._21;
	r._22 = _20*m._02 + _21*m._12 + _22*m._22;
	return r;
}

void mat3::operator *= (const mat3 &m) {
	mat3 r = (*this * m);
	*this = r;
}

vec3 mat3::operator * (const vec3 &v) const {
	return vec3(	v.x*_00 + v.y*_01 + v.z*_02,
					v.x*_10 + v.y*_11 + v.z*_12,
					v.x*_20 + v.y*_21 + v.z*_22);
}

string mat3::str() const {
	return format("(%f, %f, %f; %f, %f, %f; %f, %f, %f)", _00, _01, _02, _10, _11, _12, _20, _21, _22);
}

// kaba
void mat3::imul(const mat3 &m)
{	*this *= m;	}
mat3 mat3::mul(const mat3 &m) const
{	return *this * m;	}

vec3 mat3::mul_v(const vec3 &v) const
{	return *this * v;	}

void Matrix3Identity(mat3 &m) {
	m._00=1;	m._01=0;	m._02=0;
	m._10=0;	m._11=1;	m._12=0;
	m._20=0;	m._21=0;	m._22=1;
}

mat3 mat3::inverse() const {
	float det=   _00*_11*_22 + _01*_12*_20 + _02*_10*_21
				-_02*_11*_20 - _00*_12*_21 - _01*_10*_22;

	float *m=(float*)this;

	if (det==0){
		msg_write("Matrix3Inverse:  matrix not invertible");
		return ID;
	}
	float idet=1.0f/det;

	mat3 mo;
	mo._00= ( -_12*_21 +_11*_22 )*idet;
	mo._01= ( +_02*_21 -_01*_22 )*idet;
	mo._02= ( -_02*_11 +_01*_12 )*idet;

	mo._10= ( +_12*_20 -_10*_22 )*idet;
	mo._11= ( -_02*_20 +_00*_22 )*idet;
	mo._12= ( +_02*_10 -_00*_12 )*idet;

	mo._20= ( -_11*_20 +_10*_21 )*idet;
	mo._21= ( +_01*_20 -_00*_21 )*idet;
	mo._22= ( -_01*_10 +_00*_11 )*idet;
	return mo;
}

mat3 mat3::transpose() const {
	mat3 _m;
	_m._00=_00;	_m._01=_10;	_m._02=_20;
	_m._10=_01;	_m._11=_11;	_m._12=_21;
	_m._20=_02;	_m._21=_12;	_m._22=_22;
	return _m;
}

mat3 mat3::rotation(const vec3 &ang) {
	mat3 m;
	float sx=sinf(ang.x);
	float cx=cosf(ang.x);
	float sy=sinf(ang.y);
	float cy=cosf(ang.y);
	float sz=sinf(ang.z);
	float cz=cosf(ang.z);
	m._00= sx*sy*sz + cy*cz;	m._01= sx*sy*cz - cy*sz;	m._02= cx*sy;
	m._10= cx*sz;				m._11= cx*cz;				m._12=-sx;
	m._20= sx*cy*sz - sy*cz;	m._21= sx*cy*cz + sy*sz;	m._22= cx*cy;
	return m;
}

mat3 mat3::rotation_q(const quaternion &q) {
	mat3 m;
	m._00=1-2*q.y*q.y-2*q.z*q.z;	m._01=  2*q.x*q.y-2*q.w*q.z;	m._02=  2*q.x*q.z+2*q.w*q.y;
	m._10=  2*q.x*q.y+2*q.w*q.z;	m._11=1-2*q.x*q.x-2*q.z*q.z;	m._12=  2*q.y*q.z-2*q.w*q.x;
	m._20=  2*q.x*q.z-2*q.w*q.y;	m._21=  2*q.y*q.z+2*q.w*q.x;	m._22=1-2*q.x*q.x-2*q.y*q.y;
	return m;
}
