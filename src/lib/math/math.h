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


#include <cmath>

class color;
class vector;
class plane;
class complex;
class matrix;
class matrix3;
class quaternion;
class Ray;


#include "../base/base.h"
#include "../image/color.h"
#include "vector.h"
#include "matrix.h"
#include "matrix3.h"
#include "quaternion.h"
#include "plane.h"
#include "rect.h"
#include "complex.h"
#include "interpolation.h"
#include "random.h"
#include "ray.h"



template<class T>
T clamp(T x, T min, T max) {
	if (min >= max)
		return min;
	if (x < min)
		return min;
	if (x >= max)
		return max;
	return x;
}
template<class T>
T loop(T x, T min, T max);
template<class T>
T abs(T x) {
	if (x < 0)
		return -x;
	return x;
}
template<class T>
T sqr(T x) {
	return x * x;
}


template<>
int loop<int>(int x, int min, int max);

template<>
float loop<float>(float x, float min, float max);



int _cdecl randi(int m);
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
	return !std::isfinite(f);
#endif
}

inline bool inf_v(const vector &v)
{   return (inf_f(v.x) || inf_f(v.y) || inf_f(v.z));  }

inline bool inf_q(const quaternion &q)
{   return (inf_f(q.x) || inf_f(q.y) || inf_f(q.z) || inf_f(q.z));  }

inline bool inf_pl(plane p)
{   return (inf_v(p.n) || inf_f(p.d));  }



#endif
