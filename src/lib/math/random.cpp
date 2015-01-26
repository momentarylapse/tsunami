#include "math.h"

#define PHI 0x9e3779b9

// TODO: more possible seeds
void Random::seed(const string &s)
{
	c = 362436;
	int x = s.hash();

	Q[0] = x;
	Q[1] = x + PHI;
	Q[2] = x + PHI + PHI;

	for (int i=3; i<4096; i++)
		Q[i] = Q[i - 3] ^ Q[i - 2] ^ PHI ^ i;
}


int Random::_get()
{
	long long t, a = 18782;
	int i = 4095;
	int x, r = 0xfffffffe;
	i = (i + 1) & 4095;
	t = a * Q[i] + c;
	c = (t >> 32);
	x = (int)t + c;
	if (x < c){
		x++;
		c++;
	}
	Q[i] = r - x;
	return Q[i];
}

int Random::geti(int max)
{
	int r = _get();
	if (r < 0)
		r = - r;
	return r % max;
}

float Random::getu()
{
	float r = (float)_get();
	if (r < 0)
		r = - r;
	return r / 2147483648.0f;
}

float Random::getf(float min, float max)
{
	float r = (float)_get();
	if (r < 0)
		r = - r;
	return min + (max - min) * r / 2147483648.0f;
}

vector Random::in_ball(float r)
{
	while(true){
		vector v = vector(getf(-1, 1), getf(-1, 1), getf(-1, 1));
		if (v.length_sqr() < 1)
			return v * r;
	}
	return v_0;
}

vector Random::dir()
{
	vector v = in_ball(1);
	float l = v.length();
	if (l != 0)
		return v / l;
	return e_z;
}
