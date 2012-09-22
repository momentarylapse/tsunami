#include "types.h"

//------------------------------------------------------------------------------------------------//
//                                            vectors                                             //
//------------------------------------------------------------------------------------------------//
// real length of the vector
float vector::length() const
{
	return sqrtf( x*x + y*y + z*z );
}

// gibt die laengste Seite zurueck (="unendlich-Norm")
// (immer <= <echte Laenge>)
float vector::length_fuzzy() const
{
	float l=fabs(x);
	float a=fabs(y);
	if (a>l)
		l=a;
	a=fabs(z);
	if (a>l)
		l=a;
	return l;
}

float vector::length_sqr() const
{
	return x*x + y*y + z*z;
}

// v in cube(a,b) ?
bool vector::between(const vector &a, const vector &b) const
{
	/*if ((x>a.x)&&(x>b.x))	return false;
	if ((x<a.x)&&(x<b.x))	return false;
	if ((y>a.y)&&(y>b.y))	return false;
	if ((y<a.y)&&(y<b.y))	return false;
	if ((z>a.z)&&(z>b.z))	return false;
	if ((z<a.z)&&(z<b.z))	return false;
	return true;*/
	float f = VecDotProduct(*this - a, b - a);
	if (f < 0)
		return false;
	f /= (b - a).length_sqr();
	return (f <= 1);
}

// v = a + f*( b - a )
//   get f
float vector::factor_between(const vector &a, const vector &b) const
{
	if (a.x!=b.x)
		return ((x-a.x)/(b.x-a.x));
	else if (a.y!=b.y)
		return ((y-a.y)/(b.y-a.y));
	else if (a.z!=b.z)
		return ((z-a.z)/(b.z-a.z));
	return 0;
}

// a im Wuerfel mit Radius=d um b ?
bool vector::bounding_cube(const vector &a, float r) const
{
	if ((a.x-x>r)||(a.x-x<-r))
		return false;
	if ((a.y-y>r)||(a.y-y<-r))
		return false;
	if ((a.z-z>r)||(a.z-z<-r))
		return false;
	return true;
}

// auf die Laenge 1 bringen
void vector::normalize()
{
	float l = length();
	if (l > 0)
		*this /= l;
	else
		*this = e_z;
}

// cos( Winkel zwischen Vektoren) * Laenge1 * Laenge2
float VecDotProduct(const vector &v1, const vector &v2)
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
vector vector::transform(const matrix &m) const
{
	vector vo;
	vo.x= x*m._00 + y*m._01 + z*m._02 + m._03;
	vo.y= x*m._10 + y*m._11 + z*m._12 + m._13;
	vo.z= x*m._20 + y*m._21 + z*m._22 + m._23;
	return vo;
}

// Transformation eines Normalenvektors
// matrix * vector(x,y,z,0)
vector vector::transform_normal(const matrix &m) const
{
	vector vo;
	vo.x= x*m._00 + y*m._01 + z*m._02;
	vo.y= x*m._10 + y*m._11 + z*m._12;
	vo.z= x*m._20 + y*m._21 + z*m._22;
	return vo;
}

// Transformation eines Richtungsvektors
// matrix * vector(x,y,z,0)
vector vector::transform3(const matrix3 &m) const
{
	vector vo;
	vo.x= x*m._00 + y*m._01 + z*m._02;
	vo.y= x*m._10 + y*m._11 + z*m._12;
	vo.z= x*m._20 + y*m._21 + z*m._22;
	return vo;
}

// vector(0,0,1) wird um ang rotiert
// ZXY, da nur im Spiel-Koordinaten-System
vector vector::ang2dir() const
{
	return vector(		sinf(y)*cos(x),
					-	sinf(x),
						cosf(y)*cos(x));
}

// um welche Winkel wurde vector(0,0,1) rotiert?
vector vector::dir2ang() const
{
	return vector(	-	atan2f(y,sqrt(x*x+z*z)),
						atan2f(x,z),
						0); // too few information to get z!
}

// um welche Winkel wurde vector(0,0,1) rotiert?
//    
vector vector::dir2ang2(const vector &up) const
{
	vector right=VecCrossProduct(up,*this);
	return vector(	-	atan2f(y,sqrt(x*x+z*z)),
						atan2f(x,z),
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
	q = q2 * q1;
	return q.get_angles();
}

vector VecAngInterpolate(const vector &ang1,const vector &ang2,float t)
{
	quaternion q1,q2,r;
	QuaternionRotationV(q1,ang1);
	QuaternionRotationV(q2,ang2);
	QuaternionInterpolate(r,q1,q2,t);
	return r.get_angles();
}

// rotate a vector by an angle
vector vector::rotate(const vector &ang) const
{
	// slow...indirect
	matrix m;
	MatrixRotation(m, ang);
	return transform(m);
}

// which one is the largest coordinate of this vector
int vector::important_plane() const
{
	vector w;
	w.x=fabs(x);
	w.y=fabs(y);
	w.z=fabs(z);
	if ((w.x<=w.y)&&(w.x<=w.z))
		return 1;	// Y-Z-Ebene
	if (w.y<=w.z)
		return 2;	// X-Z-Ebene
	return 3;		// X-Y-Ebene
}

// finds an orthogonal vector to v
vector vector::ortho() const
{
	int p = important_plane();
	if (p == 0)
		return vector(y, - x, 0);
	return vector(0, z, - y);
}

// kuerzeste Entfernung von p zur Geraden(l1,l2)
float VecLineDistance(const vector &p,const vector &l1,const vector &l2)
{
	return (float)sqrt( (p-l1).length_sqr() - sqr(VecDotProduct(l2-l1,p-l1))/(l2-l1).length_sqr() );
}

// Punkt der Geraden(l1,l2), der p am naechsten ist
vector VecLineNearestPoint(const vector &p,const vector &l1,const vector &l2)
{
	vector n = l2-l1, np;
	n.normalize();
	plane pl;
	PlaneFromPointNormal(pl, p, n);
	pl.intersect_line(l1, l2, np);
	return np;
}

void vector::_min(const vector &test_partner)
{
	if (test_partner.x<x)	x=test_partner.x;
	if (test_partner.y<y)	y=test_partner.y;
	if (test_partner.z<z)	z=test_partner.z;
}

void vector::_max(const vector &test_partner)
{
	if (test_partner.x>x)	x=test_partner.x;
	if (test_partner.y>y)	y=test_partner.y;
	if (test_partner.z>z)	z=test_partner.z;
}
