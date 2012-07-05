/*
 * interpolation.h
 *
 *  Created on: 31.05.2012
 *      Author: michi
 */

#ifndef INTERPOLATION_H_
#define INTERPOLATION_H_

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

	// data input
	void Clear();
	void Add(const vector &p, float dt = 1.0f);
	void Add(const vector &p, const vector &v, float dt = 1.0f);
	void Jump(const vector &p, const vector &v);

	// interpolated output
	vector Get(float t);
	vector GetTang(float t);

	struct Part
	{
		vector pos0, pos1;
		vector vel0, vel1;
		float t0, dt;
	};

private:
	void Update();
	Type type;
	bool empty;
	bool ready;
	Array<Part> part;
	Part temp;
	float t_sum;
};


#endif /* INTERPOLATION_H_ */
