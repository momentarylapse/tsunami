#include "types.h"
#include "../file/file.h"

//------------------------------------------------------------------------------------------------//
//                                          quaternions                                           //
//------------------------------------------------------------------------------------------------//

void QuaternionIdentity(quaternion &q)
{
	q.w=1;
	q.x=q.y=q.z=0;
}

// rotation with an <angle w> and an <axis axis>
void QuaternionRotationA(quaternion &q,const vector &axis,float w)
{
	float w_half=w*0.5f;
	float s=sinf(w_half);
	q.w=cosf(w_half);
	q.x=axis.x*s;
	q.y=axis.y*s;
	q.z=axis.z*s;
}

// ZXY -> everything IN the game (world transformation)
void QuaternionRotationV(quaternion &q,const vector &ang)
{
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
}

// create a quaternion from a (rotation-) matrix
void QuaternionRotationM(quaternion &q,const matrix &m)
{
	float tr=m._00+m._11+m._22;
	float w=acosf((tr-1)/2);
	if ((w<0.00000001f)&&(w>-0.00000001f))
		QuaternionIdentity(q);
	else{
		float s=0.5f/sinf(w);
		vector n;
		n.x=(m._21-m._12)*s;
		n.y=(m._02-m._20)*s;
		n.z=(m._10-m._01)*s;
		n.normalize();
		QuaternionRotationA(q,n,w);
	}
}

// invert a quaternion rotation
void quaternion::inverse()
{
	x = -x;
	y = -y;
	z = -z;
}

// unite 2 rotations (first rotate by q1, then by q2: q = q2*q1)
void QuaternionMultiply(quaternion &q,const quaternion &q2,const quaternion &q1)
{
	quaternion _q;
	_q.w = q2.w*q1.w - q2.x*q1.x - q2.y*q1.y - q2.z*q1.z;
	_q.x = q2.w*q1.x + q2.x*q1.w + q2.y*q1.z - q2.z*q1.y;
	_q.y = q2.w*q1.y + q2.y*q1.w + q2.z*q1.x - q2.x*q1.z;
	_q.z = q2.w*q1.z + q2.z*q1.w + q2.x*q1.y - q2.y*q1.x;
	q=_q;
}

// q = q1 + t*( q2 - q1)
void QuaternionInterpolate(quaternion &q,const quaternion &q1,const quaternion &q2,float t)
{
	//msg_todo("TestMe: QuaternionInterpolate(2q) for OpenGL");
	q=q1;

	t=1-t; // ....?

	// dot product = cos angle(q1,q2)
	float c = q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;
	float t2;
	bool flip=false;
	// flip, if q1 and q2 on opposite hemispheres
	if (c<0){
		c=-c;
		flip=true;
	}
	// q1 and q2 "too equal"?
	if (c>0.9999f)
		t2=1.0f-t;
	else{
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
}

void QuaternionInterpolate(quaternion &q,const quaternion &q1,const quaternion &q2,const quaternion &q3,const quaternion &q4,float t)
{
	/*#ifdef NIX_TYPES_BY_DIRECTX9
		D3DXQUATERNION A,B,C;
		D3DXQuaternionSquadSetup(&A,&B,&C,(D3DXQUATERNION*)&q1,(D3DXQUATERNION*)&q2,(D3DXQUATERNION*)&q3,(D3DXQUATERNION*)&q4);
		D3DXQuaternionSquad((D3DXQUATERNION*)&q,(D3DXQUATERNION*)&q2,&A,&B,&C,t);
	#else*/
	q=q2;
	msg_todo("QuaternionInterpolate(4q)");
}

// convert a quaternion into 3 angles (ZXY)
vector quaternion::get_angles() const
{
	// really bad!
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
	ang.z=atan2f(v.y,v.x);
	return ang;
}

// scale the angle of the rotation
void QuaternionScale(quaternion &q,float f)
{
	float w=q.get_angle();
	if (w==0)	return;

	q.w=cosf(w*f/2);
	float factor=sinf(w*f/2)/sinf(w/2);
	q.x=q.x*factor;
	q.y=q.y*factor;
	q.z=q.z*factor;
}

// quaternion correction
void quaternion::normalize()
{
	float l=sqrtf((x*x)+(y*y)+(z*z)+(w*w));
	l=1.0f/l;
	x *= l;
	y *= l;
	z *= l;
	w *= l;
}

// the axis of our quaternion rotation
vector quaternion::get_axis() const
{
	vector ax = vector(x, y, z);
	ax.normalize();
	return ax;
}

// angle value of the quaternion
float quaternion::get_angle() const
{
	return acosf(w)*2;
}
