#include "math.h"
#include <stdlib.h>

//------------------------------------------------------------------------------------------------//
//                                             int                                                //
//------------------------------------------------------------------------------------------------//


// force <i> within a boundary by cutting it off
int clampi(int i, int min, int max)
{
	if (max <= min)
		return min;
	if (i > max)
		return max;
	if (i < min)
		return min;
	return i;
}

// force <i> within a boundary by modulo-ing
int loopi(int i, int min, int max)
{
	if (max <= min)
		return min;
	int d = max - min + 1;
	if (i < min){
		int n= (int)( (min-i-1) / d ) + 1;
		return i + d * n;
	}
	if (i > max){
		int n= (int)( (i-max-1) / d ) + 1;
		return i - d * n;
	}
	return i;
}

// random int
int randi(int m)
{
	return int((float)rand() * m / (float)RAND_MAX);
}


//------------------------------------------------------------------------------------------------//
//                                             float                                              //
//------------------------------------------------------------------------------------------------//


// square
float sqr(float f)
{
	return f * f;
}

// force <f> within a boundary by cutting it off
float clampf(float f, float min, float max)
{
	if (max >= min){
		if (f > max)
			return max;
		if (f < min)
			return min;
		return f;
	}else
		return min;
}

// force <f> within a boundary by modulo-ing
float loopf(float f,float min,float max)
{
	float d = max - min;
	if (f < min){
		int n= (int)( (min-f) / d ) + 1;
		return f + d * (float)n;
	}
	if (f>=max){
		int n= (int)( (f-max) / d ) + 1;
		return f - d * (float)n;
	}
	return f;
}

// random float
float randf(float m)
{
	return (float)rand()*m/(float)RAND_MAX;
}

