#include "vector.h"
#include "matrix.h"
#include "plane.h"
#include <math.h>

bool inf_v(const vector &v)
{   return (inf_f(v.x) || inf_f(v.y) || inf_f(v.z));  }


//------------------------------------------------------------------------------------------------//
//                                            vec2                                                //
//------------------------------------------------------------------------------------------------//



const vec2 vec2::ZERO = vec2(0, 0);
const vec2 vec2::EX = vec2(1, 0);
const vec2 vec2::EY = vec2(0, 1);

vec2::vec2(float x, float y) {
	this->x = x;
	this->y = y;
}

// assignment operators
void vec2::operator += (const vec2& v) {
	x += v.x;
	y += v.y;
}

void vec2::operator -= (const vec2& v) {
	x -= v.x;
	y -= v.y;
}

void vec2::operator *= (float f) {
	x *= f;
	y *= f;
}

void vec2::operator /= (float f) {
	x /= f;
	y /= f;
}

// unitary operator(s)
vec2 vec2::operator - () const {
	return vec2(-x, -y);
}

// binary operators
vec2 vec2::operator + (const vec2 &v) const {
	return vec2(x+v.x ,y+v.y);
}

vec2 vec2::operator - (const vec2 &v) const {
	return vec2(x-v.x, y-v.y);
}

vec2 vec2::operator * (float f) const {
	return vec2(x*f, y*f);
}

vec2 vec2::operator / (float f) const {
	return vec2(x/f, y/f);
}

bool vec2::operator == (const vec2 &v) const {
	return ((x==v.x) and (y==v.y));
}

bool vec2::operator != (const vec2 &v) const {
	return !((x==v.x) and (y==v.y));
}

string vec2::str() const {
	return format("(%f, %f)", x, y);
}

// real length of the vec2
float vec2::length() const {
	return sqrtf( x*x + y*y );
}

// scale to length 1
void vec2::normalize() {
	float l = length();
	if (l > 0)
		*this /= l;
	else
		*this = vec2::EY;
}

// scale to length 1
vec2 vec2::normalized() const {
	float l = length();
	if (l == 0)
		return vec2::EY;
	return *this / l;
}


//------------------------------------------------------------------------------------------------//
//                                            vector                                                //
//------------------------------------------------------------------------------------------------//


const vector vector::ZERO = vector(0, 0, 0);
const vector vector::EX = vector(1, 0, 0);
const vector vector::EY = vector(0, 1, 0);
const vector vector::EZ = vector(0, 0, 1);

vector::vector(float x, float y, float z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

// assignment operators
void vector::operator += (const vector& v) {
	x += v.x;
	y += v.y;
	z += v.z;
}

void vector::operator -= (const vector& v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

void vector::operator *= (float f) {
	x *= f;
	y *= f;
	z *= f;
}

void vector::operator /= (float f) {
	x /= f;
	y /= f;
	z /= f;
}

// unitary operator(s)
vector vector::operator - () const {
	return vector(-x, -y, -z);
}

// binary operators
vector vector::operator + (const vector &v) const {
	return vector(x+v.x ,y+v.y, z+v.z);
}

vector vector::operator - (const vector &v) const {
	return vector(x-v.x, y-v.y, z-v.z);
}

vector vector::operator * (float f) const {
	return vector(x*f, y*f, z*f);
}

vector vector::operator / (float f) const {
	return vector(x/f, y/f, z/f);
}

bool vector::operator == (const vector &v) const {
	return ((x==v.x) and (y==v.y) and (z==v.z));
}

bool vector::operator != (const vector &v) const {
	return !((x==v.x) and (y==v.y) and (z==v.z));
}

#if 1
float vector::operator * (const vector &v) const {
	return dot(*this, v);
}

vector vector::operator ^ (const vector &v) const {
	return cross(*this, v);
}
#endif

string vector::str() const {
	return format("(%f, %f, %f)", x, y, z);
}

// real length of the vector
float vector::length() const {
	return sqrtf( x*x + y*y + z*z );
}

// gibt die laengste Seite zurueck (="unendlich-Norm")
// (immer <= <echte Laenge>)
float vector::length_fuzzy() const {
	float l=fabs(x);
	float a=fabs(y);
	if (a>l)
		l=a;
	a=fabs(z);
	if (a>l)
		l=a;
	return l;
}

float vector::length_sqr() const {
	return x*x + y*y + z*z;
}

// v in cube(a,b) ?
bool vector::between(const vector &a, const vector &b) const {
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
float vector::factor_between(const vector &a, const vector &b) const {
	if (a.x!=b.x)
		return ((x-a.x)/(b.x-a.x));
	else if (a.y!=b.y)
		return ((y-a.y)/(b.y-a.y));
	else if (a.z!=b.z)
		return ((z-a.z)/(b.z-a.z));
	return 0;
}

// a in cube with radius=d around b ?
bool vector::bounding_cube(const vector &a, float r) const {
	if ((a.x-x>r) or (a.x-x<-r))
		return false;
	if ((a.y-y>r) or (a.y-y<-r))
		return false;
	if ((a.z-z>r) or (a.z-z<-r))
		return false;
	return true;
}

// scale to length 1
void vector::normalize() {
	float l = length();
	if (l > 0)
		*this /= l;
	else
		*this = vector::EZ;
}

// scale to length 1
vector vector::normalized() const {
	float l = length();
	if (l == 0)
		return vector::EZ;
	return *this / l;
}

// cos( Winkel zwischen Vektoren) * Laenge1 * Laenge2
float vector::dot(const vector &v1, const vector &v2) {
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

// Richtung: zu beiden orthogonal!!
// Laenge: sin( Winkel zwischen Vektoren) * Laenge1 * Laenge2
// (0,0,0) bei: ( v1 parallel v2 )
vector vector::cross(const vector &v1,const vector &v2) {
	vector v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
	return v;

}

#if 0
// Koordinaten-Transformation
// matrix * vector(x,y,z,1)
vector vector::transform(const matrix &m) const {
	return m * *this;
}

// Transformation eines Normalenvektors
// matrix * vector(x,y,z,0)
vector vector::transform_normal(const matrix &m) const {
	vector vo;
	vo.x= x*m._00 + y*m._01 + z*m._02;
	vo.y= x*m._10 + y*m._11 + z*m._12;
	vo.z= x*m._20 + y*m._21 + z*m._22;
	return vo;
}
vector vector::untransform(const matrix &m) const {
	return m.inverse() * *this;
}

// Transformation eines Richtungsvektors
// matrix * vector(x,y,z,0)
vector vector::transform3(const matrix3 &m) const {
	vector vo;
	vo.x= x*m._00 + y*m._01 + z*m._02;
	vo.y= x*m._10 + y*m._11 + z*m._12;
	vo.z= x*m._20 + y*m._21 + z*m._22;
	return vo;
}
#endif

// vector(0,0,1) wird um ang rotiert
// ZXY, da nur im Spiel-Koordinaten-System
vector vector::ang2dir() const {
	return vector(		sin(y)*cos(x),
					-	sin(x),
						cos(y)*cos(x));
}

// um welche Winkel wurde vector(0,0,1) rotiert?
vector vector::dir2ang() const {
	return vector(	-	atan2(y,sqrt(x*x+z*z)),
						atan2(x,z),
						0); // too few information to get z!
}

// um welche Winkel wurde vector(0,0,1) rotiert?
//    
vector vector::dir2ang2(const vector &up) const {
	vector right = cross(up,*this);
	return vector(	-	atan2(y,sqrt(x*x+z*z)),
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
vector VecAngAdd(const vector &ang1,const vector &ang2) {
	auto q1 = quaternion::rotation(ang1);
	auto q2 = quaternion::rotation(ang2);
	auto q = q2 * q1;
	return q.get_angles();
}

vector VecAngInterpolate(const vector &ang1,const vector &ang2,float t) {
	auto q1 = quaternion::rotation(ang1);
	auto q2 = quaternion::rotation(ang2);
	auto r = quaternion::interpolate(q1,q2,t);
	return r.get_angles();
}

// rotate a vector by an angle
vector vector::rotate(const vector &ang) const {
	// slow...indirect
	matrix m = matrix::rotation(ang);
	return transform(m);
}
#endif

// which one is the largest coordinate of this vector
int vector::important_plane() const {
	vector w;
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
vector vector::ortho() const {
	int p = important_plane();
	if (p == 2)
		return vector(y, - x, 0);
	else if (p == 1)
		return vector(z, 0, - x);
	return vector(0, z, - y);
}

// kuerzeste Entfernung von p zur Geraden(l1,l2)
float VecLineDistance(const vector &p,const vector &l1,const vector &l2) {
	return (float)sqrt( (p-l1).length_sqr() - sqr(vector::dot(l2-l1,p-l1))/(l2-l1).length_sqr() );
}

// Punkt der Geraden(l1,l2), der p am naechsten ist
vector VecLineNearestPoint(const vector &p,const vector &l1,const vector &l2) {
	vector n = l2-l1, np;
	n.normalize();
	plane pl = plane::from_point_normal(p, n);
	pl.intersect_line(l1, l2, np);
	return np;
}

void vector::_min(const vector &test_partner) {
	if (test_partner.x<x)	x=test_partner.x;
	if (test_partner.y<y)	y=test_partner.y;
	if (test_partner.z<z)	z=test_partner.z;
}

void vector::_max(const vector &test_partner) {
	if (test_partner.x>x)	x=test_partner.x;
	if (test_partner.y>y)	y=test_partner.y;
	if (test_partner.z>z)	z=test_partner.z;
}


float _vec_length_(const vector &v) {
	return sqrt(vector::dot(v, v));
}

float _vec_length_fuzzy_(const vector &v) {
	float x = fabs(v.x);
	float y = fabs(v.y);
	float z = fabs(v.z);
	float xy = (x > y) ? x : y;
	return (xy > z) ? xy : z;
}

void _vec_normalize_(vector &v) {
	float inv_norm = 1.0f / v.length();
	v *= inv_norm;
}

bool _vec_between_(const vector &v,const vector &a,const vector &b) {
	float ff = _vec_factor_between_(v, a, b);
	return (ff >= 0) and (ff <= 1);
}

float _vec_factor_between_(const vector &v,const vector &a,const vector &b) {
	return vector::dot(v-a, b-a) / vector::dot(b-a, b-a);
}







//------------------------------------------------------------------------------------------------//
//                                            vec4                                                //
//------------------------------------------------------------------------------------------------//




const vec4 vec4::ZERO = vec4(0, 0, 0, 0);
const vec4 vec4::EX = vec4(1, 0, 0, 0);
const vec4 vec4::EY = vec4(0, 1, 0, 0);
const vec4 vec4::EZ = vec4(0, 0, 1, 0);
const vec4 vec4::EW = vec4(0, 0, 0, 1);

vec4::vec4(float x, float y, float z, float w) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

// assignment operators
void vec4::operator += (const vec4& v) {
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
}

void vec4::operator -= (const vec4& v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
}

void vec4::operator *= (float f) {
	x *= f;
	y *= f;
	z *= f;
	w *= f;
}

void vec4::operator /= (float f) {
	x /= f;
	y /= f;
	z /= f;
	w /= f;
}

// unitary operator(s)
vec4 vec4::operator - () const {
	return vec4(-x, -y, -z, -w);
}

// binary operators
vec4 vec4::operator + (const vec4 &v) const {
	return vec4(x+v.x ,y+v.y, z+v.z, w+v.w);
}

vec4 vec4::operator - (const vec4 &v) const {
	return vec4(x-v.x, y-v.y, z-v.z, w-v.w);
}

vec4 vec4::operator * (float f) const {
	return vec4(x*f, y*f, z*f, w*f);
}

vec4 vec4::operator / (float f) const {
	return vec4(x/f, y/f, z/f, w/f);
}

bool vec4::operator == (const vec4 &v) const {
	return ((x==v.x) and (y==v.y) and (z==v.z) and (w==v.w));
}

bool vec4::operator != (const vec4 &v) const {
	return !((x==v.x) and (y==v.y) and (z==v.z) and (w==v.w));
}

string vec4::str() const {
	return format("(%f, %f, %f, %f)", x, y, z, w);
}

// real length of the vec4
float vec4::length() const {
	return sqrtf( x*x + y*y + z*z + w*w );
}

// scale to length 1
void vec4::normalize() {
	float l = length();
	if (l > 0)
		*this /= l;
	else
		*this = vec4::EW;
}

// scale to length 1
vec4 vec4::normalized() const {
	float l = length();
	if (l == 0)
		return vec4::EW;
	return *this / l;
}





int vec4::argmin() const {
	int n = 0;
	float m = (*this)[0];
	for (int i=1; i<3; i++)
		if ((*this)[i] < m) {
			n = i;
			m = (*this)[i];
		}
	return n;
}

int vec4::argmax() const {
	int n = 0;
	float m = (*this)[0];
	for (int i=1; i<3; i++)
		if ((*this)[i] > m) {
			n = i;
			m = (*this)[i];
		}
	return n;
}

float vec4::sum() const {
	return x + y + z + w;
}

float &vec4::operator[](int index) {
	auto vv = &x;
	return vv[index];
}

float vec4::operator[](int index) const {
	auto vv = &x;
	return vv[index];
}



int ivec4::find(int x) const {
	if (i == x)
		return 0;
	if (j == x)
		return 1;
	if (k == x)
		return 2;
	if (l == x)
		return 3;
	return -1;
}

int &ivec4::operator[](int index) {
	auto vv = &i;
	return vv[index];
}

int ivec4::operator[](int index) const {
	auto vv = &i;
	return vv[index];
}




