/*
 * interpolation.h
 *
 *  Created on: 31.05.2012
 *      Author: michi
 */

#pragma once


#include "../base/base.h"

template<class T>
class Interpolator {
public:
	enum Type{
		TYPE_LERP,
		TYPE_CUBIC_SPLINE,
		TYPE_CUBIC_SPLINE_NOTANG,
		TYPE_ANGULAR_LERP,
	};
	explicit Interpolator(Type type);
	void _cdecl __init__();
	void _cdecl setType(const string &type);

	// data input
	void _cdecl clear();
	void _cdecl add(const T &p, float dt = 1.0f);
	void _cdecl add2(const T &p, const T &v, float dt = 1.0f);
	void _cdecl add3(const T &p, const T &v, float weight, float dt = 1.0f);
	void _cdecl addv(const T p, float dt = 1.0f);
	void _cdecl add2v(const T p, const T v, float dt = 1.0f);
	void _cdecl add3v(const T p, const T v, float weight, float dt = 1.0f);
	void _cdecl jump(const T &p, const T &v);
	void _cdecl jumpv(const T p, const T v);
	void _cdecl close(float dt = 1.0f);

	void normalize();
	float getDuration();

	// interpolated output
	T _cdecl get(float t);
	T _cdecl get_derivative(float t);
	Array<T> _cdecl get_list(Array<float> &t);

	struct Part {
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
	bool closed;
};

// points = [p0, dp0, p1, dp1]
template<class T>
T cubic_spline(const Array<T>& points, float t) {
	if (points.num < 4)
		return T{};
	const float tt = t*t;
	const float ttt = tt*t;
	return (2*ttt-3*tt+1) * points[0] + (ttt-2*tt+t) * points[1] + (-2*ttt+3*tt) * points[2] + (ttt-tt) * points[3];
}

template<class T>
T cubic_spline_d(const Array<T>& points, float t) {
	if (points.num < 4)
		return T{};
	const float tt = t*t;
	return (6*tt-6*t) * points[0] + (3*tt-4*t+1) * points[1] + (-6*tt+6*t) * points[2] + (3*tt-2*t) * points[3];
}

