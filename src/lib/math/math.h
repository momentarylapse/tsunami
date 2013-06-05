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
struct Ray;


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



#endif
