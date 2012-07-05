/*
 * interpolation.cpp
 *
 *  Created on: 31.05.2012
 *      Author: michi
 */

#include "types.h"

Interpolator::Interpolator(Type _type)
{
	type = _type;
	Clear();
}



void Interpolator::Clear()
{
	part.clear();
	empty = true;
	ready = false;
	t_sum = 0;
	temp.t0 = 0;
}



void Interpolator::Add(const vector & p, float dt)
{
	Add(p, v0, dt);
}



void Interpolator::Add(const vector & p, const vector & v, float dt)
{
	if (!empty){
		temp.pos1 = p;
		temp.vel1 = v;
		temp.t0 = t_sum;
		part.add(temp);
		t_sum += temp.dt;
	}
	temp.pos0 = p;
	temp.vel0 = v;
	temp.dt = dt;
	empty = false;
}



void Interpolator::Jump(const vector & p, const vector & v)
{
	temp.pos0 = p;
	temp.vel0 = v;
	empty = false;
}



void Interpolator::Update()
{
	if (type == TYPE_CUBIC_SPLINE_NOTANG){
		type = TYPE_CUBIC_SPLINE;
		part[0].vel0 = (part[0].pos1 - part[0].pos0);
		part.back().vel1 = (part.back().pos1 - part.back().pos0);
		for (int i=1;i<part.num;i++){
			vector v = (part[i].pos1 - part[i - 1].pos0) / 2;
			part[i - 1].vel1 = v;
			part[i    ].vel0 = v;
		};
	}
	ready = true;
}



inline vector _inter_lerp_(const Interpolator::Part &p, float t)
{	return (1 - t) * p.pos0 + t * p.pos1;	}

inline vector _inter_lerp_tang_(const Interpolator::Part &p, float t)
{	return p.pos1 - p.pos0;	}

inline vector _inter_cubic_spline_(const Interpolator::Part &p, float t)
{
	float tt = t * t;
	float ttt = tt * t;
	return (  2 * ttt - 3 * tt + 1) * p.pos0 +
	       (      ttt - 2 * tt + t) * p.vel0 +
	       (- 2 * ttt + 3 * tt    ) * p.pos1 +
	       (      ttt -     tt    ) * p.vel1;
}

inline vector _inter_cubic_spline_tang_(const Interpolator::Part &p, float t)
{
	float tt = t * t;
	return (  6 * tt - 6 * t    ) * p.pos0 +
	       (  3 * tt - 4 * t + 1) * p.vel0 +
	       (- 6 * tt + 6 * t    ) * p.pos1 +
	       (  3 * tt - 2 * t    ) * p.vel1;
}

inline vector _inter_angular_lerp_(const Interpolator::Part &p, float t)
{
	quaternion q0, q1, q;
	QuaternionRotationV(q0, p.pos0);
	QuaternionRotationV(q1, p.pos1);
	QuaternionInterpolate(q, q0, q1, t);
	return QuaternionToAngle(q);
}



vector Interpolator::Get(float t)
{
	if (!ready)
		Update();
	if (part.num > 0){
		t = clampf(t, 0, 0.99999f);
		float f_index = (float)part.num * t;
		int index = (int)f_index;
		t = f_index - index;
		if (type == TYPE_LERP)
			return _inter_lerp_(part[index], t);
		if (type == TYPE_CUBIC_SPLINE)
			return _inter_cubic_spline_(part[index], t);
		if (type == TYPE_ANGULAR_LERP)
			return _inter_angular_lerp_(part[index], t);
		return part[index].pos0;
	}
	return v0;
}



vector Interpolator::GetTang(float t)
{
	if (!ready)
		Update();
	if (part.num > 0){
		t = clampf(t, 0, 0.99999f);
		float f_index = (float)part.num * t;
		int index = (int)f_index;
		t = f_index - index;
		if (type == TYPE_LERP)
			return _inter_lerp_tang_(part[index], t);
		if (type == TYPE_CUBIC_SPLINE)
			return _inter_cubic_spline_tang_(part[index], t);
	}
	return v0;
}



