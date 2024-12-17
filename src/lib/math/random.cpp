#include "random.h"
#include "vec3.h"
#include "../os/date.h"

#define PHI 0x9e3779b9

static constexpr int N = 4096;

Random::Random() {
	Q.resize(N);
	c = 0;
	auto d = Date::now();
	seed(d.format("%c") + i2s(d.milli_second));
}

void Random::__assign__(Random *other) {
	*this = *other;
}

// TODO: more possible seeds
void Random::seed(const string &s) {
	c = 362436;
	const int x = s.hash();

	Q[0] = x;
	Q[1] = x + PHI;
	Q[2] = x + PHI + PHI;

	for (int i=3; i<N; i++)
		Q[i] = Q[i - 3] ^ Q[i - 2] ^ PHI ^ i;
}


int Random::_get() {
	int64 a = 18782;
	int i = N - 1;
	unsigned int r = 0xfffffffe;
	i = (i + 1) & (N - 1);
	int64 t = a * Q[i] + c;
	c = (t >> 32);
	unsigned int x = (unsigned int)t + c;
	if (x < c) {
		x ++;
		c ++;
	}
	Q[i] = r - x;
	return (int)Q[i];
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
	const float x = uniform(-1,1);
	const float y = uniform(0, 2*pi);

	float xx = 0;
	if (x > 0) {
		xx = sqrt( -2.0f * log(x));
	} else if (x < 0) {
		xx = -sqrt( -2.0f * log(-x));
	}

	const float a = xx * cos(y);
	//float b = xx * sin(y);

	return mean + a * stddev;
}

vec3 Random::in_ball(float r) {
	while (true) {
		const vec3 v = vec3(uniform(-1, 1), uniform(-1, 1), uniform(-1, 1));
		if (v.length_sqr() < 1)
			return v * r;
	}
	return v_0;
}

vec3 Random::dir() {
	const vec3 v = in_ball(1);
	const float l = v.length();
	if (l != 0)
		return v / l;
	return vec3::EZ;
}
