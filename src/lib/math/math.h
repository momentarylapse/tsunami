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
class vec3;
class plane;
class complex;
class mat4;
class mat3;
class quaternion;
class Ray;


#include "../base/base.h"



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
T sign(T x) {
	if (x < 0)
		return -1;
	if (x == 0)
		return 0;
	return 1;
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

inline bool inf_f(float f) {
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




#endif
