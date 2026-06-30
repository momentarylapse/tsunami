
#pragma once


#include "math.h"

struct Box;

class Random {
public:
	Random();

	void seed(const string &s);
	int _get();
	int _int(int max);
	float uniform01();
	float uniform(float min, float max);
	float normal(float mean, float stddev);

	vec3 in_ball(float r);
	vec3 dir();
	vec3 in_box(const Box& b);

private:
	Array<unsigned int> Q;
	unsigned int c;
};

