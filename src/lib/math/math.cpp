#include "math.h"
#include <stdlib.h>


//------------------------------------------------------------------------------------------------//
//                                             int                                                //
//------------------------------------------------------------------------------------------------//


// force <i> within a boundary by modulo-ing
template<>
int loop<int>(int i, int min, int max) {
	if (max <= min)
		return min;
	int d = max - min;
	if (i < min){
		int n = (int)((min-i) / d) + 1;
		return i + d * n;
	}
	if (i >= max){
		int n = (int)((i-max) / d) + 1;
		return i - d * n;
	}
	return i;
}

// random int
int randi(int m) {
	return int((float)rand() * m / (float)RAND_MAX);
}


//------------------------------------------------------------------------------------------------//
//                                             float                                              //
//------------------------------------------------------------------------------------------------//



// force <f> within a boundary by modulo-ing
template<>
float loop<float>(float f,float min,float max) {
	float d = max - min;
	if (f < min) {
		int n = (int)( (min-f) / d ) + 1;
		return f + d * (float)n;
	} if (f >= max) {
		int n= (int)( (f-max) / d ) + 1;
		return f - d * (float)n;
	}
	return f;
}

// random float
float randf(float m) {
	return (float)rand()*m/(float)RAND_MAX;
}

