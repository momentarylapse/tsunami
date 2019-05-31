#include "math.h"
#include "../file/file.h"

//------------------------------------------------------------------------------------------------//
//                                          3x3-matrices                                          //
//------------------------------------------------------------------------------------------------//


const float f_m3_id[9] = { 1,0,0 , 0,1,0 , 0,0,1 };
const matrix3 matrix3::ID = matrix3(f_m3_id);

matrix3::matrix3(const float f[9])
{
	for (int i=0;i<9;i++)
		e[i]=f[i];
}


matrix3 matrix3::operator + (const matrix3 &m) const
{
	matrix3 r;
	for (int i=0;i<9;i++)
		r.e[i]=e[i]+m.e[i];
	return r;
}

matrix3 matrix3::operator - (const matrix3 &m) const
{
	matrix3 r;
	for (int i=0;i<9;i++)
		r.e[i]=e[i]-m.e[i];
	return r;
}

matrix3 matrix3::operator * (float f) const
{
	matrix3 r = *this;
	r *= f;
	return r;
}

matrix3 matrix3::operator *= (float f)
{
	for (int i=0;i<9;i++)
		e[i] *= f;
	return *this;
}

matrix3 matrix3::operator / (float f) const
{
	matrix3 r = *this;
	r /= f;
	return r;
}

matrix3 matrix3::operator /= (float f)
{
	for (int i=0;i<9;i++)
		e[i] /= f;
	return *this;
}

matrix3 matrix3::operator * (const matrix3 &m) const
{
	matrix3 r;
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

void matrix3::operator *= (const matrix3 &m)
{
	matrix3 r = (*this * m);
	*this = r;
}

vector matrix3::operator * (const vector &v) const
{
	return vector(	v.x*_00 + v.y*_01 + v.z*_02,
					v.x*_10 + v.y*_11 + v.z*_12,
					v.x*_20 + v.y*_21 + v.z*_22);
}

string matrix3::str() const
{
	return format("(%f, %f, %f; %f, %f, %f; %f, %f, %f)", _00, _01, _02, _10, _11, _12, _20, _21, _22);
}

// kaba
void matrix3::imul(const matrix3 &m)
{	*this *= m;	}
matrix3 matrix3::mul(const matrix3 &m) const
{	return *this * m;	}

vector matrix3::mul_v(const vector &v) const
{	return *this * v;	}

void Matrix3Identity(matrix3 &m)
{
	m._00=1;	m._01=0;	m._02=0;
	m._10=0;	m._11=1;	m._12=0;
	m._20=0;	m._21=0;	m._22=1;
}

void Matrix3Inverse(matrix3 &mo,const matrix3 &mi)
{
	float det=   mi._00*mi._11*mi._22 + mi._01*mi._12*mi._20 + mi._02*mi._10*mi._21
				-mi._02*mi._11*mi._20 - mi._00*mi._12*mi._21 - mi._01*mi._10*mi._22;

	float *m=(float*)&mi;

	if (det==0){
		msg_write("Matrix3Inverse:  matrix not invertible");
		Matrix3Identity(mo);
		return;
	}
	float idet=1.0f/det;

	mo._00= ( -mi._12*mi._21 +mi._11*mi._22 )*idet;
	mo._01= ( +mi._02*mi._21 -mi._01*mi._22 )*idet;
	mo._02= ( -mi._02*mi._11 +mi._01*mi._12 )*idet;

	mo._10= ( +mi._12*mi._20 -mi._10*mi._22 )*idet;
	mo._11= ( -mi._02*mi._20 +mi._00*mi._22 )*idet;
	mo._12= ( +mi._02*mi._10 -mi._00*mi._12 )*idet;

	mo._20= ( -mi._11*mi._20 +mi._10*mi._21 )*idet;
	mo._21= ( +mi._01*mi._20 -mi._00*mi._21 )*idet;
	mo._22= ( -mi._01*mi._10 +mi._00*mi._11 )*idet;
}

void Matrix3Transpose(matrix3 &mo,const matrix3 &mi)
{
	matrix3 _m;
	_m._00=mi._00;	_m._01=mi._10;	_m._02=mi._20;
	_m._10=mi._01;	_m._11=mi._11;	_m._12=mi._21;
	_m._20=mi._02;	_m._21=mi._12;	_m._22=mi._22;
	mo=_m;
}

void Matrix3Rotation(matrix3 &m,const vector &ang)
{
	float sx=sinf(ang.x);
	float cx=cosf(ang.x);
	float sy=sinf(ang.y);
	float cy=cosf(ang.y);
	float sz=sinf(ang.z);
	float cz=cosf(ang.z);
	m._00= sx*sy*sz + cy*cz;	m._01= sx*sy*cz - cy*sz;	m._02= cx*sy;
	m._10= cx*sz;				m._11= cx*cz;				m._12=-sx;
	m._20= sx*cy*sz - sy*cz;	m._21= sx*cy*cz + sy*sz;	m._22= cx*cy;
}

void Matrix3RotationQ(matrix3 &m,const quaternion &q)
{
	m._00=1-2*q.y*q.y-2*q.z*q.z;	m._01=  2*q.x*q.y-2*q.w*q.z;	m._02=  2*q.x*q.z+2*q.w*q.y;
	m._10=  2*q.x*q.y+2*q.w*q.z;	m._11=1-2*q.x*q.x-2*q.z*q.z;	m._12=  2*q.y*q.z-2*q.w*q.x;
	m._20=  2*q.x*q.z-2*q.w*q.y;	m._21=  2*q.y*q.z+2*q.w*q.x;	m._22=1-2*q.x*q.x-2*q.y*q.y;
}
