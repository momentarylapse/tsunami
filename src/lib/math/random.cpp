#include "random.h"
#include "vector.h"
#include "../file/file.h"

#define PHI 0x9e3779b9

Random::Random() {
	auto d = Date::now();
	seed(d.format("%c") + i2s(d.milli_second));
}

void Random::__init__() {
	new(this) Random;
}

void Random::__assign__(Random *other) {
	*this = *other;
}

// TODO: more possible seeds
void Random::seed(const string &s) {
	c = 362436;
	int x = s.hash();

	Q[0] = x;
	Q[1] = x + PHI;
	Q[2] = x + PHI + PHI;

	for (int i=3; i<4096; i++)
		Q[i] = Q[i - 3] ^ Q[i - 2] ^ PHI ^ i;
}


int Random::_get() {
	long long t, a = 18782;
	int i = 4095;
	int x, r = 0xfffffffe;
	i = (i + 1) & 4095;
	t = a * Q[i] + c;
	c = (t >> 32);
	x = (int)t + c;
	if (x < c) {
		x++;
		c++;
	}
	Q[i] = r - x;
	return Q[i];
}

int Random::_int(int max) {
	int r = _get();
	if (r < 0)
		r = - r;
	return r % max;
}

float Random::uniform01() {
	return uniform(0, 1);
}

float Random::uniform(float min, float max) {
	float r = (float)_get();
	if (r < 0)
		r = - r;
	return min + (max - min) * r / 2147483648.0f;
}

float Random::normal(float mean, float stddev) {
	float x = uniform(-1,1);
	float y = uniform(0, 2*pi);

	float xx = 0;
	if (x > 0) {
		xx = sqrt( -2.0f * log(x));
	} else if (x < 0) {
		xx = -sqrt( -2.0f * log(-x));
	}

	float a = xx * cos(y);
	float b = xx * sin(y);

	return mean + a * stddev;
}

vector Random::in_ball(float r) {
	while(true) {
		vector v = vector(uniform(-1, 1), uniform(-1, 1), uniform(-1, 1));
		if (v.length_sqr() < 1)
			return v * r;
	}
	return v_0;
}

vector Random::dir() {
	vector v = in_ball(1);
	float l = v.length();
	if (l != 0)
		return v / l;
	return vector::EZ;
}
