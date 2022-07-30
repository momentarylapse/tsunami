#include "vec3.h"
#include "vec2.h"
#include "mat4.h"
#include "plane.h"
#include <math.h>

bool inf_v(const vec3 &v)
{   return (inf_f(v.x) || inf_f(v.y) || inf_f(v.z));  }



const vec3 vec3::ZERO = vec3(0, 0, 0);
const vec3 vec3::EX = vec3(1, 0, 0);
const vec3 vec3::EY = vec3(0, 1, 0);
const vec3 vec3::EZ = vec3(0, 0, 1);

vec3::vec3(float x, float y, float z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

vec3::vec3(const vec2 &xy, float z) {
	this->x = xy.x;
	this->y = xy.y;
	this->z = z;
}

vec2 &vec3::xy() {
	return *(vec2*)&x;
}

// assignment operators
void vec3::operator += (const vec3& v) {
	x += v.x;
	y += v.y;
	z += v.z;
}

void vec3::operator -= (const vec3& v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

void vec3::operator *= (float f) {
	x *= f;
	y *= f;
	z *= f;
}

void vec3::operator /= (float f) {
	x /= f;
	y /= f;
	z /= f;
}

// unitary operator(s)
vec3 vec3::operator - () const {
	return vec3(-x, -y, -z);
}

// binary operators
vec3 vec3::operator + (const vec3 &v) const {
	return vec3(x+v.x ,y+v.y, z+v.z);
}

vec3 vec3::operator - (const vec3 &v) const {
	return vec3(x-v.x, y-v.y, z-v.z);
}

vec3 vec3::operator * (float f) const {
	return vec3(x*f, y*f, z*f);
}

vec3 vec3::operator / (float f) const {
	return vec3(x/f, y/f, z/f);
}

bool vec3::operator == (const vec3 &v) const {
	return ((x==v.x) and (y==v.y) and (z==v.z));
}

bool vec3::operator != (const vec3 &v) const {
	return !((x==v.x) and (y==v.y) and (z==v.z));
}

#if 1
float vec3::operator * (const vec3 &v) const {
	return dot(*this, v);
}

vec3 vec3::operator ^ (const vec3 &v) const {
	return cross(*this, v);
}
#endif

string vec3::str() const {
	return format("(%f, %f, %f)", x, y, z);
}

// real length of the vector
float vec3::length() const {
	return sqrtf( x*x + y*y + z*z );
}

// gibt die laengste Seite zurueck (="unendlich-Norm")
// (immer <= <echte Laenge>)
float vec3::length_fuzzy() const {
	float l=fabs(x);
	float a=fabs(y);
	if (a>l)
		l=a;
	a=fabs(z);
	if (a>l)
		l=a;
	return l;
}

float vec3::length_sqr() const {
	return x*x + y*y + z*z;
}

// v in cube(a,b) ?
bool vec3::between(const vec3 &a, const vec3 &b) const {
	/*if ((x>a.x)&&(x>b.x))	return false;
	if ((x<a.x)&&(x<b.x))	return false;
	if ((y>a.y)&&(y>b.y))	return false;
	if ((y<a.y)&&(y<b.y))	return false;
	if ((z>a.z)&&(z>b.z))	return false;
	if ((z<a.z)&&(z<b.z))	return false;
	return true;*/
	float f = dot(*this - a, b - a);
	if (f < 0)
		return false;
	f /= (b - a).length_sqr();
	return (f <= 1);
}

// v = a + f*( b - a )
//   get f
float vec3::factor_between(const vec3 &a, const vec3 &b) const {
	if (a.x!=b.x)
		return ((x-a.x)/(b.x-a.x));
	else if (a.y!=b.y)
		return ((y-a.y)/(b.y-a.y));
	else if (a.z!=b.z)
		return ((z-a.z)/(b.z-a.z));
	return 0;
}

// a in cube with radius=d around b ?
bool vec3::bounding_cube(const vec3 &a, float r) const {
	if ((a.x-x>r) or (a.x-x<-r))
		return false;
	if ((a.y-y>r) or (a.y-y<-r))
		return false;
	if ((a.z-z>r) or (a.z-z<-r))
		return false;
	return true;
}

// scale to length 1
void vec3::normalize() {
	float l = length();
	if (l > 0)
		*this /= l;
	else
		*this = vec3::EZ;
}

// scale to length 1
vec3 vec3::normalized() const {
	float l = length();
	if (l == 0)
		return vec3::EZ;
	return *this / l;
}

// cos( Winkel zwischen Vektoren) * Laenge1 * Laenge2
float vec3::dot(const vec3 &v1, const vec3 &v2) {
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

// Richtung: zu beiden orthogonal!!
// Laenge: sin( Winkel zwischen Vektoren) * Laenge1 * Laenge2
// (0,0,0) bei: ( v1 parallel v2 )
vec3 vec3::cross(const vec3 &v1,const vec3 &v2) {
	vec3 v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
	return v;

}

#if 0
// Koordinaten-Transformation
// matrix * vector(x,y,z,1)
vec3 vec3::transform(const matrix &m) const {
	return m * *this;
}

// Transformation eines Normalenvektors
// matrix * vector(x,y,z,0)
vec3 vec3::transform_normal(const matrix &m) const {
	vec3 vo;
	vo.x= x*m._00 + y*m._01 + z*m._02;
	vo.y= x*m._10 + y*m._11 + z*m._12;
	vo.z= x*m._20 + y*m._21 + z*m._22;
	return vo;
}
vec3 vec3::untransform(const matrix &m) const {
	return m.inverse() * *this;
}

// Transformation eines Richtungsvektors
// matrix * vector(x,y,z,0)
vec3 vec3::transform3(const matrix3 &m) const {
	vec3 vo;
	vo.x= x*m._00 + y*m._01 + z*m._02;
	vo.y= x*m._10 + y*m._11 + z*m._12;
	vo.z= x*m._20 + y*m._21 + z*m._22;
	return vo;
}
#endif

// vector(0,0,1) wird um ang rotiert
// ZXY, da nur im Spiel-Koordinaten-System
vec3 vec3::ang2dir() const {
	return vec3(		sin(y)*cos(x),
					-	sin(x),
						cos(y)*cos(x));
}

// um welche Winkel wurde vector(0,0,1) rotiert?
vec3 vec3::dir2ang() const {
	return vec3(	-	atan2(y,sqrt(x*x+z*z)),
						atan2(x,z),
						0); // too few information to get z!
}

// um welche Winkel wurde vector(0,0,1) rotiert?
//    
vec3 vec3::dir2ang2(const vec3 &up) const {
	vec3 right = cross(up,*this);
	return vec3(	-	atan2(y,sqrt(x*x+z*z)),
						atan2(x,z),
						atan2(right.y,up.y)); // atan2( < up, (0,1,0) >, < right, (0,1,0) > )    where: right = up x dir
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


#if 0
// adds two angles (juxtaposition of rotations)
vec3 VecAngAdd(const vec3 &ang1,const vec3 &ang2) {
	auto q1 = quaternion::rotation(ang1);
	auto q2 = quaternion::rotation(ang2);
	auto q = q2 * q1;
	return q.get_angles();
}

vec3 VecAngInterpolate(const vec3 &ang1,const vec3 &ang2,float t) {
	auto q1 = quaternion::rotation(ang1);
	auto q2 = quaternion::rotation(ang2);
	auto r = quaternion::interpolate(q1,q2,t);
	return r.get_angles();
}

// rotate a vector by an angle
vec3 vec3::rotate(const vec3 &ang) const {
	// slow...indirect
	matrix m = matrix::rotation(ang);
	return transform(m);
}
#endif

// which one is the largest coordinate of this vector
int vec3::important_plane() const {
	vec3 w;
	w.x = fabs(x);
	w.y = fabs(y);
	w.z = fabs(z);
	if ((w.x<=w.y) and (w.x<=w.z))
		return 0;	// Y-Z-Ebene
	if (w.y<=w.z)
		return 1;	// X-Z-Ebene
	return 2;		// X-Y-Ebene
}

// finds an orthogonal vector to v
vec3 vec3::ortho() const {
	int p = important_plane();
	if (p == 2)
		return vec3(y, - x, 0);
	else if (p == 1)
		return vec3(z, 0, - x);
	return vec3(0, z, - y);
}

// kuerzeste Entfernung von p zur Geraden(l1,l2)
float VecLineDistance(const vec3 &p,const vec3 &l1,const vec3 &l2) {
	return (float)sqrt( (p-l1).length_sqr() - sqr(vec3::dot(l2-l1,p-l1))/(l2-l1).length_sqr() );
}

// Punkt der Geraden(l1,l2), der p am naechsten ist
vec3 VecLineNearestPoint(const vec3 &p,const vec3 &l1,const vec3 &l2) {
	vec3 n = l2-l1, np;
	n.normalize();
	plane pl = plane::from_point_normal(p, n);
	pl.intersect_line(l1, l2, np);
	return np;
}

void vec3::_min(const vec3 &test_partner) {
	if (test_partner.x<x)	x=test_partner.x;
	if (test_partner.y<y)	y=test_partner.y;
	if (test_partner.z<z)	z=test_partner.z;
}

void vec3::_max(const vec3 &test_partner) {
	if (test_partner.x>x)	x=test_partner.x;
	if (test_partner.y>y)	y=test_partner.y;
	if (test_partner.z>z)	z=test_partner.z;
}


float _vec_length_(const vec3 &v) {
	return sqrt(vec3::dot(v, v));
}

float _vec_length_fuzzy_(const vec3 &v) {
	float x = fabs(v.x);
	float y = fabs(v.y);
	float z = fabs(v.z);
	float xy = (x > y) ? x : y;
	return (xy > z) ? xy : z;
}

void _vec_normalize_(vec3 &v) {
	float inv_norm = 1.0f / v.length();
	v *= inv_norm;
}

bool _vec_between_(const vec3 &v,const vec3 &a,const vec3 &b) {
	float ff = _vec_factor_between_(v, a, b);
	return (ff >= 0) and (ff <= 1);
}

float _vec_factor_between_(const vec3 &v,const vec3 &a,const vec3 &b) {
	return vec3::dot(v-a, b-a) / vec3::dot(b-a, b-a);
}





