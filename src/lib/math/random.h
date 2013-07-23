
#ifndef RANDOM_H_
#define RANDOM_H_

class Random
{
public:
	void _cdecl seed(const string &s);
	int _cdecl _get();
	int _cdecl geti(int max);
	float _cdecl getu();
	float _cdecl getf(float min, float max);

	vector _cdecl in_ball(float r);
	vector _cdecl dir();

private:
	int Q[4096];
	int c;
};


#endif
