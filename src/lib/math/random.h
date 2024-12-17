
#pragma once


#include "math.h"

class Random {
public:
	Random();
	void _cdecl __assign__(Random *other);

	void _cdecl seed(const string &s);
	int _cdecl _get();
	int _cdecl _int(int max);
	float _cdecl uniform01();
	float _cdecl uniform(float min, float max);
	float _cdecl normal(float mean, float stddev);

	vec3 _cdecl in_ball(float r);
	vec3 _cdecl dir();

private:
	Array<unsigned int> Q;
	unsigned int c;
};

