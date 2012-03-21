#include "types.h"

//------------------------------------------------------------------------------------------------//
//                                            vectors                                             //
//------------------------------------------------------------------------------------------------//
// real length of the vector
float VecLength(const vector &v)
{
	return sqrtf( v*v );
}

// gibt die laengste Seite zurueck (="unendlich-Norm")
// (immer <= <echte Laenge>)
float VecLengthFuzzy(const vector &v)
{
	float l=fabs(v.x);
	float a=fabs(v.y);
	if (a>l)
		l=a;
	a=fabs(v.z);
	if (a>l)
		l=a;
	return l;
}

float VecLengthSqr(const vector &v)
{
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

// v in cube(a,b) ?
bool VecBetween(const vector &v,const vector &a,const vector &b)
{
	/*if ((v.x>a.x)&&(v.x>b.x))	return false;
	if ((v.x<a.x)&&(v.x<b.x))	return false;
	if ((v.y>a.y)&&(v.y>b.y))	return false;
	if ((v.y<a.y)&&(v.y<b.y))	return false;
	if ((v.z>a.z)&&(v.z>b.z))	return false;
	if ((v.z<a.z)&&(v.z<b.z))	return false;
	return true;*/
	float f=VecDotProduct(v-a,b-a);
	if (f<0)
		return false;
	f/=VecDotProduct(b-a,b-a);
	return (f<=1);
}

// v = a + f*( b - a )
//   get f
float VecFactorBetween(const vector &v,const vector &a,const vector &b)
{
	if (a.x!=b.x)
		return ((v.x-a.x)/(b.x-a.x));
	else if (a.y!=b.y)
		return ((v.y-a.y)/(b.y-a.y));
	else if (a.z!=b.z)
		return ((v.z-a.z)/(b.z-a.z));
	return 0;
}

// a im Wuerfel mit Radius=d um b ?
bool VecBoundingBox(const vector &a,const vector &b,float d)
{
	if ((a.x-b.x>d)||(a.x-b.x<-d))
		return false;
	if ((a.y-b.y>d)||(a.y-b.y<-d))
		return false;
	if ((a.z-b.z>d)||(a.z-b.z<-d))
		return false;
	return true;
}

// auf die Laenge 1 bringen
void VecNormalize(vector &v)
{
	float l=sqrtf(v * v);
	if (l > 0)
		v /= l;
	else
		v = e_z;
}

// cos( Winkel zwischen Vektoren) * Laenge1 * Laenge2
float VecDotProduct(const vector &v1,const vector &v2)
{
	return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z;
}

// Richtung: zu beiden orthogonal!!
// Laenge: sin( Winkel zwischen Vektoren) * Laenge1 * Laenge2
// (0,0,0) bei: ( v1 parallel v2 )
vector VecCrossProduct(const vector &v1,const vector &v2)
{
	vector v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
	return v;

}

// Koordinaten-Transformation
// matrix * vector(x,y,z,1)
void VecTransform(vector &vo,const matrix &m,const vector &vi)
{
	vector vi_=vi;
	vo.x= vi_.x*m._00 + vi_.y*m._01 + vi_.z*m._02 + m._03;
	vo.y= vi_.x*m._10 + vi_.y*m._11 + vi_.z*m._12 + m._13;
	vo.z= vi_.x*m._20 + vi_.y*m._21 + vi_.z*m._22 + m._23;
}

// Transformation eines Normalenvektors
// matrix * vector(x,y,z,0)
void VecNormalTransform(vector &vo,const matrix &m,const vector &vi)
{
	vector vi_=vi;
	vo.x= vi_.x*m._00 + vi_.y*m._01 + vi_.z*m._02;
	vo.y= vi_.x*m._10 + vi_.y*m._11 + vi_.z*m._12;
	vo.z= vi_.x*m._20 + vi_.y*m._21 + vi_.z*m._22;
}

// Transformation eines Richtungsvektors
// matrix * vector(x,y,z,0)
void VecTransform3(vector &vo,const matrix3 &m,const vector &vi)
{
	vector vi_=vi;
	vo.x= vi_.x*m._00 + vi_.y*m._01 + vi_.z*m._02;
	vo.y= vi_.x*m._10 + vi_.y*m._11 + vi_.z*m._12;
	vo.z= vi_.x*m._20 + vi_.y*m._21 + vi_.z*m._22;
}

// vector(0,0,1) wird um ang rotiert
// ZXY, da nur im Spiel-Koordinaten-System
vector VecAng2Dir(const vector &ang)
{
	return vector(		sinf(ang.y)*cos(ang.x),
					-	sinf(ang.x),
						cosf(ang.y)*cos(ang.x));
}

// um welche Winkel wurde vector(0,0,1) rotiert?
vector VecDir2Ang(const vector &dir)
{
	return vector(	-	atan2f(dir.y,sqrt(dir.x*dir.x+dir.z*dir.z)),
						atan2f(dir.x,dir.z),
						0); // too few information to get z!
}

// um welche Winkel wurde vector(0,0,1) rotiert?
//    
vector VecDir2Ang2(const vector &dir,const vector &up)
{
	vector right=VecCrossProduct(up,dir);
	return vector(	-	atan2f(dir.y,sqrt(dir.x*dir.x+dir.z*dir.z)),
						atan2f(dir.x,dir.z),
						atan2f(right.y,up.y)); // atan2( < up, (0,1,0) >, < right, (0,1,0) > )    where: right = up x dir
/*	// aus den 3 Basis-Vektoren eine Basis-Wechsel-Matrix erzeugen
	matrix m;
	m._00=right.x;	m._01=up.x;	m._02=dir.x;	m._03=0;
	m._10=right.y;	m._11=up.y;	m._12=dir.y;	m._13=0;
	m._20=right.z;	m._21=up.z;	m._22=dir.z;	m._23=0;
	m._30=0;		m._31=0;	m._32=0;		m._33=1;
	quaternion q;
	msg_todo("VecDir2Ang2 fuer OpenGL");
	QuaternionRotationM(q,m);
	msg_db_out(1,"");
	return QuaternionToAngle(q);*/
}

// adds two angles (juxtaposition of rotations)
vector VecAngAdd(const vector &ang1,const vector &ang2)
{
	quaternion q,q1,q2;
	QuaternionRotationV(q1,ang1);
	QuaternionRotationV(q2,ang2);
	QuaternionMultiply(q,q2,q1);
	return QuaternionToAngle(q);
}

vector VecAngInterpolate(const vector &ang1,const vector &ang2,float t)
{
	quaternion q1,q2,r;
	QuaternionRotationV(q1,ang1);
	QuaternionRotationV(q2,ang2);
	QuaternionInterpolate(r,q1,q2,t);
	return QuaternionToAngle(r);
}

// rotate a vector by an angle
vector VecRotate(const vector &v, const vector &ang)
{
	// slow...indirect
	matrix m;
	MatrixRotation(m, ang);
	vector r;
	VecNormalTransform(r, m, v);
	return r;
}

// which one is the largest coordinate of this vector
int VecImportantPlane(const vector &v)
{
	vector w;
	w.x=fabs(v.x);
	w.y=fabs(v.y);
	w.z=fabs(v.z);
	if ((w.x<=w.y)&&(w.x<=w.z))
		return 1;	// Y-Z-Ebene
	if (w.y<=w.z)
		return 2;	// X-Z-Ebene
	return 3;		// X-Y-Ebene
}

// finds an orthogonal vector to v
vector _cdecl VecOrtho(const vector &v)
{
	int p = VecImportantPlane(v);
	if (p == 0)
		return vector(v.y, - v.x, 0);
	return vector(0, v.z, - v.y);
}

// kuerzeste Entfernung von p zur Geraden(l1,l2)
float VecLineDistance(const vector &p,const vector &l1,const vector &l2)
{
	return (float)sqrt( VecLengthSqr(p-l1) - sqr(VecDotProduct(l2-l1,p-l1))/VecLengthSqr(l2-l1) );
}

// Punkt der Geraden(l1,l2), der p am naechsten ist
vector VecLineNearestPoint(const vector &p,const vector &l1,const vector &l2)
{
	vector n=l2-l1,np;
	VecNormalize(n);
	plane pl;
	PlaneFromPointNormal(pl,p,n);
	PlaneIntersectLine(np,pl,l1,l2);
	return np;
}

void VecMin(vector &v,const vector &test_partner)
{
	if (test_partner.x<v.x)	v.x=test_partner.x;
	if (test_partner.y<v.y)	v.y=test_partner.y;
	if (test_partner.z<v.z)	v.z=test_partner.z;
}

void VecMax(vector &v,const vector &test_partner)
{
	if (test_partner.x>v.x)	v.x=test_partner.x;
	if (test_partner.y>v.y)	v.y=test_partner.y;
	if (test_partner.z>v.z)	v.z=test_partner.z;
}
