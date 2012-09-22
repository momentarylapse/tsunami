/*----------------------------------------------------------------------------*\
| types                                                                        |
| -> mathematical types and functions                                          |
|   -> vector, matrix, matrix3, quaternion, plane, color                       |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2011.01.15 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _TYPES_INCLUDED_
#define _TYPES_INCLUDED_


#include <math.h>

struct color;
struct vector;
struct plane;
struct complex;
struct matrix;
struct matrix3;
struct quaternion;


#include "../base/base.h"
#include "color.h"
#include "vector.h"
#include "matrix.h"
#include "matrix3.h"
#include "quaternion.h"
#include "plane.h"
#include "rect.h"
#include "complex.h"
#include "interpolation.h"




// ints
int _cdecl clampi(int i, int min, int max);
int _cdecl loopi(int i, int min, int max);
int _cdecl randi(int m);

// floats
float sqr(float f);
float _cdecl clampf(float f, float min, float max);
float _cdecl loopf(float f, float min, float max);
float _cdecl randf(float m);

const float pi = 3.141592654f;


// faster functions

inline bool inf_f(float f)
{
	/*int t=*(int*)&f;
	int m=0x7f000000;
	if ((t&m)==m)   return true;
	return (f!=f);*/
#ifdef OS_WINDOWS
	return false;
#else
	return !isfinite(f);
#endif
}

inline bool inf_v(vector v)
{   return (inf_f(v.x) || inf_f(v.y) || inf_f(v.z));  }

inline bool inf_pl(plane p)
{   return (inf_v(p.n) || inf_f(p.d));  }

inline float _vec_length_(const vector &v)
{	return sqrt(v*v);	}

inline float _vec_length_fuzzy_(const vector &v)
{
	float x=fabs(v.x);
	float y=fabs(v.y);
	float z=fabs(v.z);
	float xy=(x>y)?x:y;
	return (xy>z)?xy:z;
}

inline void _vec_normalize_(vector &v)
{	float inv_norm = 1.0f / sqrtf(v*v); v *= inv_norm;	}

inline bool _vec_between_(const vector &v,const vector &a,const vector &b)
{
	if ((v.x>a.x)&&(v.x>b.x))	return false;
	if ((v.x<a.x)&&(v.x<b.x))	return false;
	if ((v.y>a.y)&&(v.y>b.y))	return false;
	if ((v.y<a.y)&&(v.y<b.y))	return false;
	if ((v.z>a.z)&&(v.z>b.z))	return false;
	if ((v.z<a.z)&&(v.z<b.z))	return false;
	return true;
}

inline float _vec_factor_between_(const vector &v,const vector &a,const vector &b)
{	return ((v-a)*(b-a)) / ((b-a)*(b-a));	}

inline void _get_bary_centric_(const vector &p,const plane &pl,const vector &a,const vector &b,const vector &c,float &f,float &g)
{
	vector ba=b-a,ca=c-a;
	vector pvec=pl.n^ca;
	float det=ba*pvec;
	vector pa;
	if (det>0)
		pa=p-a;
	else{
		pa=a-p;
		det=-det;
	}
	f=pa*pvec;
	vector qvec=pa^ba;
	g=pl.n*qvec;
	float inv_det=1.0f/det;
	f*=inv_det;
	g*=inv_det;
}

inline void _plane_from_point_normal_(plane &pl,const vector &p,const vector &n)
{
	pl.n=n;
	pl.d=-(n*p);
}

inline bool _plane_intersect_line_(vector &cp,const plane &pl,const vector &l1,const vector &l2)
{
	float e=pl.n*l1;
	float f=pl.n*l2;
	if (e==f) // parallel?
		return false;
	float t=-(pl.d+f)/(e-f);
	//if ((t>=0)&&(t<=1)){
		//cp = l1 + t*(l2-l1);
		cp = l2 + t*(l1-l2);
	return true;
}

inline float _plane_distance_(const plane &pl,const vector &p)
{	return pl.n*p + pl.d;	}

inline vector *_matrix_get_translation_(const matrix &m)
{	return (vector*)&m._03;	} // (_03, _13, _23) happens to be aligned the right way...




#endif
