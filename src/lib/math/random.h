
#ifndef RANDOM_H_
#define RANDOM_H_

class Random
{
public:
	void seed(const string &s);
	int _get();
	int geti(int max);
	float getu();
	float getf(float min, float max);

	vector in_ball(float r);
	vector dir();

private:
	int Q[4096];
	int c;
};


#endif
