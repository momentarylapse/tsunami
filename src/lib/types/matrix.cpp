#include "types.h"
#include "../file/file.h"

// ZXY -> Objekte und Figur-Teile / Modell-Transformationen
// der Vektor nach vorne (0,0,1) wird
// 1. um die z-Achse gedreht (um sich selbst)
// 2. um die x-Achse gedreht (nach oben oder unten "genickt")
// 3. um die y-Achse gedreht (nach links oder rechts)
// (alle Drehungen um Achsen, die nicht veraendert werden!!!)

// YXZ -> Kamera-/Projektions-Transformation
// der Vektor nach vorne (0,0,1) wird
// ... aber in die jeweils andere Richtung (-ang)

// YXZ-Matrix ist die Inverse zur ZXY-Matrix!!!


//------------------------------------------------------------------------------------------------//
//                                           matrices                                             //
//------------------------------------------------------------------------------------------------//

#define _ps(a,b,i,j)	(a.__e[0][i]*b.__e[j][0] + a.__e[1][i]*b.__e[j][1] + a.__e[2][i]*b.__e[j][2] + a.__e[3][i]*b.__e[j][3])

// combining two transformation matrices (first do m1, then m2:   m = m2 * m1 )
void MatrixMultiply(matrix &m,const matrix &m2,const matrix &m1)
{
	// m_ij = (sum k) m2_ik * m1_kj
	matrix _m;
	_m._00=_ps(m2,m1,0,0);	_m._01=_ps(m2,m1,0,1);	_m._02=_ps(m2,m1,0,2);	_m._03=_ps(m2,m1,0,3);
	_m._10=_ps(m2,m1,1,0);	_m._11=_ps(m2,m1,1,1);	_m._12=_ps(m2,m1,1,2);	_m._13=_ps(m2,m1,1,3);
	_m._20=_ps(m2,m1,2,0);	_m._21=_ps(m2,m1,2,1);	_m._22=_ps(m2,m1,2,2);	_m._23=_ps(m2,m1,2,3);
	_m._30=_ps(m2,m1,3,0);	_m._31=_ps(m2,m1,3,1);	_m._32=_ps(m2,m1,3,2);	_m._33=_ps(m2,m1,3,3);
	m=_m;
}

// combining two transformation matrices (first do m1, then m2:   m = m2 * m1 )
matrix MatrixMultiply2(const matrix &m2, const matrix &m1)
{
	// m_ij = (sum k) m2_ik * m1_kj
	matrix _m;
	_m._00=_ps(m2,m1,0,0);	_m._01=_ps(m2,m1,0,1);	_m._02=_ps(m2,m1,0,2);	_m._03=_ps(m2,m1,0,3);
	_m._10=_ps(m2,m1,1,0);	_m._11=_ps(m2,m1,1,1);	_m._12=_ps(m2,m1,1,2);	_m._13=_ps(m2,m1,1,3);
	_m._20=_ps(m2,m1,2,0);	_m._21=_ps(m2,m1,2,1);	_m._22=_ps(m2,m1,2,2);	_m._23=_ps(m2,m1,2,3);
	_m._30=_ps(m2,m1,3,0);	_m._31=_ps(m2,m1,3,1);	_m._32=_ps(m2,m1,3,2);	_m._33=_ps(m2,m1,3,3);
	return _m;
}

static const float _IdentityMatrix[16]={	1,0,0,0,	0,1,0,0,	0,0,1,0,	0,0,0,1	};

// identity (no transformation: m*v=v)
void MatrixIdentity(matrix &m)
{
	m = _IdentityMatrix;
	//memcpy(&m,&_IdentityMatrix,sizeof(matrix));
	/*
	m._00=1;	m._01=0;	m._02=0;	m._03=0;
	m._10=0;	m._11=1;	m._12=0;	m._13=0;
	m._20=0;	m._21=0;	m._22=1;	m._23=0;
	m._30=0;	m._31=0;	m._32=0;	m._33=1;*/
}

float _Determinant(float m[16])
{
	return
		m[12]*m[9]*m[6]*m[3]-
		m[8]*m[13]*m[6]*m[3]-
		m[12]*m[5]*m[10]*m[3]+
		m[4]*m[13]*m[10]*m[3]+
		m[8]*m[5]*m[14]*m[3]-
		m[4]*m[9]*m[14]*m[3]-
		m[12]*m[9]*m[2]*m[7]+
		m[8]*m[13]*m[2]*m[7]+
		m[12]*m[1]*m[10]*m[7]-
		m[0]*m[13]*m[10]*m[7]-
		m[8]*m[1]*m[14]*m[7]+
		m[0]*m[9]*m[14]*m[7]+
		m[12]*m[5]*m[2]*m[11]-
		m[4]*m[13]*m[2]*m[11]-
		m[12]*m[1]*m[6]*m[11]+
		m[0]*m[13]*m[6]*m[11]+
		m[4]*m[1]*m[14]*m[11]-
		m[0]*m[5]*m[14]*m[11]-
		m[8]*m[5]*m[2]*m[15]+
		m[4]*m[9]*m[2]*m[15]+
		m[8]*m[1]*m[6]*m[15]-
		m[0]*m[9]*m[6]*m[15]-
		m[4]*m[1]*m[10]*m[15]+
		m[0]*m[5]*m[10]*m[15];
}

// inverting the transformation
void MatrixInverse(matrix &mo,const matrix &mi)
{
	float *m=(float*)&mi;
	float *i=(float*)&mo;
	float x=_Determinant(m);

	/*msg_write("Matrix Inverse");
	mout(mi);
	msg_write(f2s(x,3));*/

	if (x==0){
		msg_write("MatrixInverse:  matrix not invertible");
		MatrixIdentity(mo);
		return;
	}

	i[0]= (-m[13]*m[10]*m[7] +m[9]*m[14]*m[7] +m[13]*m[6]*m[11] -m[5]*m[14]*m[11] -m[9]*m[6]*m[15] +m[5]*m[10]*m[15])/x;
	i[4]= ( m[12]*m[10]*m[7] -m[8]*m[14]*m[7] -m[12]*m[6]*m[11] +m[4]*m[14]*m[11] +m[8]*m[6]*m[15] -m[4]*m[10]*m[15])/x;
	i[8]= (-m[12]*m[9]* m[7] +m[8]*m[13]*m[7] +m[12]*m[5]*m[11] -m[4]*m[13]*m[11] -m[8]*m[5]*m[15] +m[4]*m[9]* m[15])/x;
	i[12]=( m[12]*m[9]* m[6] -m[8]*m[13]*m[6] -m[12]*m[5]*m[10] +m[4]*m[13]*m[10] +m[8]*m[5]*m[14] -m[4]*m[9]* m[14])/x;
	i[1]= ( m[13]*m[10]*m[3] -m[9]*m[14]*m[3] -m[13]*m[2]*m[11] +m[1]*m[14]*m[11] +m[9]*m[2]*m[15] -m[1]*m[10]*m[15])/x;
	i[5]= (-m[12]*m[10]*m[3] +m[8]*m[14]*m[3] +m[12]*m[2]*m[11] -m[0]*m[14]*m[11] -m[8]*m[2]*m[15] +m[0]*m[10]*m[15])/x;
	i[9]= ( m[12]*m[9]* m[3] -m[8]*m[13]*m[3] -m[12]*m[1]*m[11] +m[0]*m[13]*m[11] +m[8]*m[1]*m[15] -m[0]*m[9]* m[15])/x;
	i[13]=(-m[12]*m[9]* m[2] +m[8]*m[13]*m[2] +m[12]*m[1]*m[10] -m[0]*m[13]*m[10] -m[8]*m[1]*m[14] +m[0]*m[9]* m[14])/x;
	i[2]= (-m[13]*m[6]* m[3] +m[5]*m[14]*m[3] +m[13]*m[2]*m[7]  -m[1]*m[14]*m[7] -m[5]*m[2]*m[15] +m[1]*m[6]* m[15])/x;
	i[6]= ( m[12]*m[6]* m[3] -m[4]*m[14]*m[3] -m[12]*m[2]*m[7]  +m[0]*m[14]*m[7] +m[4]*m[2]*m[15] -m[0]*m[6]* m[15])/x;
	i[10]=(-m[12]*m[5]* m[3] +m[4]*m[13]*m[3] +m[12]*m[1]*m[7]  -m[0]*m[13]*m[7] -m[4]*m[1]*m[15] +m[0]*m[5]* m[15])/x;
	i[14]=( m[12]*m[5]* m[2] -m[4]*m[13]*m[2] -m[12]*m[1]*m[6]  +m[0]*m[13]*m[6] +m[4]*m[1]*m[14] -m[0]*m[5]* m[14])/x;
	i[3]= ( m[9]* m[6]* m[3] -m[5]*m[10]*m[3] -m[9]* m[2]*m[7]  +m[1]*m[10]*m[7] +m[5]*m[2]*m[11] -m[1]*m[6]* m[11])/x;
	i[7]= (-m[8]* m[6]* m[3] +m[4]*m[10]*m[3] +m[8]* m[2]*m[7]  -m[0]*m[10]*m[7] -m[4]*m[2]*m[11] +m[0]*m[6]* m[11])/x;
	i[11]=( m[8]* m[5]* m[3] -m[4]*m[9]* m[3] -m[8]* m[1]*m[7]  +m[0]*m[9]* m[7] +m[4]*m[1]*m[11] -m[0]*m[5]* m[11])/x;
	i[15]=(-m[8]* m[5]* m[2] +m[4]*m[9]* m[2] +m[8]* m[1]*m[6]  -m[0]*m[9]* m[6] -m[4]*m[1]*m[10] +m[0]*m[5]* m[10])/x;
}

// transposes a matrix
void MatrixTranspose(matrix &mo,const matrix &mi)
{
	matrix _m;
	_m._00=mi._00;	_m._01=mi._10;	_m._02=mi._20;	_m._03=mi._30;
	_m._10=mi._01;	_m._11=mi._11;	_m._12=mi._21;	_m._13=mi._31;
	_m._20=mi._02;	_m._21=mi._12;	_m._22=mi._22;	_m._23=mi._32;
	_m._30=mi._03;	_m._31=mi._13;	_m._32=mi._23;	_m._33=mi._33;
	mo=_m;
}

// translation by a vector ( m * v = v + t )
void MatrixTranslation(matrix &m,const vector &t)
{
	m._00=1;	m._01=0;	m._02=0;	m._03=t.x;
	m._10=0;	m._11=1;	m._12=0;	m._13=t.y;
	m._20=0;	m._21=0;	m._22=1;	m._23=t.z;
	m._30=0;	m._31=0;	m._32=0;	m._33=1;
}

// Rotation um die X-Achse (nach unten)
void MatrixRotationX(matrix &m,float w)
{
	float sw=sinf(w);
	float cw=cosf(w);
	m._00=1;	m._01=0;	m._02=0;	m._03=0;
	m._10=0;	m._11=cw;	m._12=-sw;	m._13=0;
	m._20=0;	m._21=sw;	m._22=cw;	m._23=0;
	m._30=0;	m._31=0;	m._32=0;	m._33=1;
}

// Rotation um die Y-Achse (nach rechts)
void MatrixRotationY(matrix &m,float w)
{
	float sw=sinf(w);
	float cw=cosf(w);
	m._00=cw;	m._01=0;	m._02=sw;	m._03=0;
	m._10=0;	m._11=1;	m._12=0;	m._13=0;
	m._20=-sw;	m._21=0;	m._22=cw;	m._23=0;
	m._30=0;	m._31=0;	m._32=0;	m._33=1;
}

// Rotation um die Z-Achse (gegen den Uhrzeigersinn)
void MatrixRotationZ(matrix &m,float w)
{
	float sw=sinf(w);
	float cw=cosf(w);
	m._00=cw;	m._01=-sw;	m._02=0;	m._03=0;
	m._10=sw;	m._11=cw;	m._12=0;	m._13=0;
	m._20=0;	m._21=0;	m._22=1;	m._23=0;
	m._30=0;	m._31=0;	m._32=0;	m._33=1;
}

// ZXY -> fuer alles IM Spiel
void MatrixRotation(matrix &m,const vector &ang)
{
	/*matrix x,y,z;
	MatrixRotationX(x,ang.x);
	MatrixRotationY(y,ang.y);
	MatrixRotationZ(z,ang.z);
	// m=y*x*z
	MatrixMultiply(m,y,x);
	MatrixMultiply(m,m,z);*/
	float sx=sinf(ang.x);
	float cx=cosf(ang.x);
	float sy=sinf(ang.y);
	float cy=cosf(ang.y);
	float sz=sinf(ang.z);
	float cz=cosf(ang.z);
	m._00= sx*sy*sz + cy*cz;	m._01= sx*sy*cz - cy*sz;	m._02= cx*sy;	m._03=0;
	m._10= cx*sz;				m._11= cx*cz;				m._12=-sx;		m._13=0;
	m._20= sx*cy*sz - sy*cz;	m._21= sx*cy*cz + sy*sz;	m._22= cx*cy;	m._23=0;
	m._30= 0;					m._31= 0;					m._32=0;		m._33=1;
}

// ZXY -> fuer alles IM Spiel
matrix MatrixRotation2(const vector &ang)
{
	matrix m;
	/*matrix x,y,z;
	MatrixRotationX(x,ang.x);
	MatrixRotationY(y,ang.y);
	MatrixRotationZ(z,ang.z);
	// m=y*x*z
	MatrixMultiply(m,y,x);
	MatrixMultiply(m,m,z);*/
	float sx=sinf(ang.x);
	float cx=cosf(ang.x);
	float sy=sinf(ang.y);
	float cy=cosf(ang.y);
	float sz=sinf(ang.z);
	float cz=cosf(ang.z);
	m._00= sx*sy*sz + cy*cz;	m._01= sx*sy*cz - cy*sz;	m._02= cx*sy;	m._03=0;
	m._10= cx*sz;				m._11= cx*cz;				m._12=-sx;		m._13=0;
	m._20= sx*cy*sz - sy*cz;	m._21= sx*cy*cz + sy*sz;	m._22= cx*cy;	m._23=0;
	m._30= 0;					m._31= 0;					m._32=0;		m._33=1;
	return m;
}

// YXZ -> fuer Kamera-Rotationen
// ist die Inverse zu MatrixRotation!!
void MatrixRotationView(matrix &m,const vector &ang)
{
	/*matrix x,y,z;
	MatrixRotationX(x,-ang.x);
	MatrixRotationY(y,-ang.y);
	MatrixRotationZ(z,-ang.z);
	// z*x*y
	MatrixMultiply(m,z,x);
	MatrixMultiply(m,m,y);*/
	float sx=sinf(ang.x);
	float cx=cosf(ang.x);
	float sy=sinf(ang.y);
	float cy=cosf(ang.y);
	float sz=sinf(ang.z);
	float cz=cosf(ang.z);
	// the transposed (=inverted) of MatrixView
	m._00= sx*sy*sz + cy*cz;	m._01= cx*sz;	m._02= sx*cy*sz - sy*cz;	m._03=0;
	m._10= sx*sy*cz - cy*sz;	m._11= cx*cz;	m._12= sx*cy*cz + sy*sz;	m._13=0;
	m._20= cx*sy;				m._21=-sx;		m._22= cx*cy;				m._23=0;
	m._30= 0;					m._31= 0;		m._32=0;					m._33=1;
}

// Rotations-Matrix aus Quaternion erzeugen
void MatrixRotationQ(matrix &m,const quaternion &q)
{
	m._00=1-2*q.y*q.y-2*q.z*q.z;	m._01=  2*q.x*q.y-2*q.w*q.z;	m._02=  2*q.x*q.z+2*q.w*q.y;	m._03=0;
	m._10=  2*q.x*q.y+2*q.w*q.z;	m._11=1-2*q.x*q.x-2*q.z*q.z;	m._12=  2*q.y*q.z-2*q.w*q.x;	m._13=0;
	m._20=  2*q.x*q.z-2*q.w*q.y;	m._21=  2*q.y*q.z+2*q.w*q.x;	m._22=1-2*q.x*q.x-2*q.y*q.y;	m._23=0;
	m._30=0;						m._31=0;						m._32=0;						m._33=1;
	/* [ 1 - 2y2 - 2z2        2xy - 2wz        2xz + 2wy
	         2xy + 2wz    1 - 2x2 - 2z2        2yz - 2wx
		     2xz - 2wy        2yz + 2wx    1 - 2x2 - 2y2 ] */
}

// scale orthogonally in 3 dimensions
void MatrixScale(matrix &m,float fx,float fy,float fz)
{
	m._00=fx;	m._01=0;	m._02=0;	m._03=0;
	m._10=0;	m._11=fy;	m._12=0;	m._13=0;
	m._20=0;	m._21=0;	m._22=fz;	m._23=0;
	m._30=0;	m._31=0;	m._32=0;	m._33=1;
}

// create a transformation that reflects at a <plane pl>
void MatrixReflect(matrix &m,const plane &pl)
{
	vector n = pl.n;
	vector p=-n*pl.d;
	// mirror: matrix s from transforming the basis vectors:
	//    e_i' = e_i - 2 < n, e_i >
	//     or thinking of it as a tensor product (projection): s = id - 2n (x) n
	// translation by p: t_p
	// complete reflection is: r = t_p * s * t_(-p) = t_(2p) * s
	m._00=1-2*n.x*n.x;	m._01= -2*n.y*n.x;	m._02= -2*n.z*n.x;	m._03=2*p.x;
	m._10= -2*n.x*n.y;	m._11=1-2*n.y*n.y;	m._12= -2*n.z*n.y;	m._13=2*p.y;
	m._20= -2*n.x*n.z;	m._21= -2*n.y*n.z;	m._22=1-2*n.z*n.z;	m._23=2*p.z;
	m._30=0;			m._31=0;			m._32=0;			m._33=1;
	//msg_todo("TestMe: MatrixReflect");
}

