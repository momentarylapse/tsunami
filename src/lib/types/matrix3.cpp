#include "types.h"
#include "../file/file.h"

//------------------------------------------------------------------------------------------------//
//                                          3x3-matrices                                          //
//------------------------------------------------------------------------------------------------//

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
