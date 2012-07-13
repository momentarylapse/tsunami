/*
 * interpolation.h
 *
 *  Created on: 31.05.2012
 *      Author: michi
 */

#ifndef INTERPOLATION_H_
#define INTERPOLATION_H_

template<class T>
class Interpolator
{
public:
	enum Type{
		TYPE_LERP,
		TYPE_CUBIC_SPLINE,
		TYPE_CUBIC_SPLINE_NOTANG,
		TYPE_ANGULAR_LERP,
	};
	Interpolator(Type type);
	void __init__();
	void set_type(const string &_type);

	// data input
	void clear();
	void add(const T &p, float dt = 1.0f);
	void add2(const T &p, const T &v, float dt = 1.0f);
	void add3(const T &p, const T &v, float weight, float dt = 1.0f);
	void jump(const T &p, const T &v);

	// interpolated output
	T get(float t);
	T get_tang(float t);
	Array<T> get_list(Array<float> &t);

	struct Part
	{
		T pos0, pos1;
		T vel0, vel1;
		float t0, dt;
		float weight0, weight1;
	};

private:
	void update();
	int canonize(float &t);
	void print();
	Type type;
	bool empty;
	bool ready;
	Array<Part> part;
	Part temp;
	float t_sum;
};


#endif /* INTERPOLATION_H_ */
