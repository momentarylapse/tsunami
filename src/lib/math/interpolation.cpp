/*
 * interpolation.cpp
 *
 *  Created on: 31.05.2012
 *      Author: michi
 */

#include "math.h"
#include "../file/file.h"




template<class T>
Interpolator<T>::Interpolator(Type _type)
{
	type = _type;
	clear();
}

template<class T>
void Interpolator<T>::__init__()
{
	new(this) Interpolator<T>(TYPE_LERP);
}

template<class T>
void Interpolator<T>::setType(const string &_type)
{
	if (_type == "lerp")
		type = TYPE_LERP;
	else if (_type == "cubic-spline")
		type = TYPE_CUBIC_SPLINE;
	else if (_type == "cubic-spline-notang")
		type = TYPE_CUBIC_SPLINE_NOTANG;
	else if (_type == "angular-lerp")
		type = TYPE_ANGULAR_LERP;
	else{
		type = TYPE_LERP;
		msg_error("unknown interpolator type: " + _type);
	}
	clear();
}


template<class T>
void Interpolator<T>::clear()
{
	part.clear();
	empty = true;
	ready = false;
	t_sum = 0;
	temp.t0 = 0;
	closed = false;
}

template<class T>
void Interpolator<T>::close(float dt)
{
	add(part[0].pos0, dt);
	closed = true;
}



template<class T>
T _inter_zero_();

template<>
inline float _inter_zero_<float>()
{	return 0;	}

template<>
inline vector _inter_zero_<vector>()
{	return v_0;	}

template<>
inline complex _inter_zero_<complex>()
{	return complex(0,0);	}

template<>
inline quaternion _inter_zero_<quaternion>()
{	return quaternion(0,v_0);	}



template<class T>
void Interpolator<T>::add(const T &p, float dt)
{	add3(p, _inter_zero_<T>(), 1, dt);	}


template<class T>
void Interpolator<T>::add2(const T &p, const T &v, float dt)
{	add3(p, v, 1, dt);	}


template<class T>
void Interpolator<T>::add3(const T &p, const T &v, float weight, float dt)
{
	if (!empty){
		temp.pos1 = p;
		temp.vel1 = v;
		temp.weight1 = weight;
		temp.t0 = t_sum;
		temp.dt = dt;
		float ww = temp.weight0 + temp.weight1;
		temp.weight0 /= ww;
		temp.weight1 /= ww;
		part.add(temp);
		t_sum += temp.dt;
	}
	temp.pos0 = p;
	temp.vel0 = v;
	temp.weight0 = weight;
	temp.dt = 0;
	empty = false;
}

template<class T>
void Interpolator<T>::addv(const T p, float dt)
{	add3(p, _inter_zero_<T>(), 1, dt);	}


template<class T>
void Interpolator<T>::add2v(const T p, const T v, float dt)
{	add3(p, v, 1, dt);	}


template<class T>
void Interpolator<T>::add3v(const T p, const T v, float weight, float dt)
{	add3(p, v, weight, dt);	}


template<class T>
void Interpolator<T>::jump(const T &p, const T &v)
{
	temp.pos0 = p;
	temp.vel0 = v;
	temp.weight0 = 1;
	empty = false;
}

template<class T>
void Interpolator<T>::jumpv(const T p, const T v)
{
	temp.pos0 = p;
	temp.vel0 = v;
	temp.weight0 = 1;
	empty = false;
}


template<class T>
void Interpolator<T>::update()
{
	if (type == TYPE_CUBIC_SPLINE_NOTANG){
		if (closed){
			T v = (part[0].pos1 - part.back().pos0) / (part.back().dt + part[0].dt);
			part.back().vel1 = v;
			part[0].vel0 = v;
		}else{
			part[0].vel0 = (part[0].pos1 - part[0].pos0) / part[0].dt;
			part.back().vel1 = (part.back().pos1 - part.back().pos0) / part.back().dt;
		}
		for (int i=1;i<part.num;i++){
			T v = (part[i].pos1 - part[i - 1].pos0) / (part[i - 1].dt + part[i].dt);
			part[i - 1].vel1 = v;
			part[i    ].vel0 = v;
		}
	}
	ready = true;
}

template<class T>
void Interpolator<T>::normalize()
{
	for (Part &p : part){
		p.t0 /= t_sum;
		p.dt /= t_sum;
		p.vel0 *= t_sum;
		p.vel1 *= t_sum;
	}
	t_sum = 1;
}


template<class T>
inline T _inter_lerp_(const typename Interpolator<T>::Part &p, float t)
{	return (1 - t) * p.pos0 + t * p.pos1;	}

template<class T>
inline T _inter_lerp_tang_(const typename Interpolator<T>::Part &p, float t)
{	return p.vel0;	}


template<class T>
inline T _inter_cubic_spline_(const typename Interpolator<T>::Part &p, float t)
{
	/*float tt = t * t;
	float ttt = tt * t;
	return (  2 * ttt - 3 * tt + 1) * p.pos0 +
	       (      ttt - 2 * tt + t) * p.vel0 * p.dt +
	       (- 2 * ttt + 3 * tt    ) * p.pos1 +
	       (      ttt -     tt    ) * p.vel1 * p.dt;*/
	float tt = 1 - t;
	T pp0 = p.pos0 + (p.vel0 * p.dt) / 3;
	T pp1 = p.pos1 - (p.vel1 * p.dt) / 3;
	float l0 = p.weight0 * 2;
	float l1 = p.weight1 * 2;
	float b0 = tt * tt * tt / l1;
	float b1 = 3 * tt * tt * t;
	float b2 = 3 * tt * t * t;
	float b3 = t * t * t / l0;

	return (p.pos0 * b0 + pp0 * b1 + pp1 * b2 + p.pos1 * b3) / (b0 + b1 + b2 + b3);

}

template<class T>
inline T _inter_cubic_spline_tang_(const typename Interpolator<T>::Part &p, float t)
{
	float tt = t * t;
	return (  6 * tt - 6 * t    ) * p.pos0 / p.dt +
	       (  3 * tt - 4 * t + 1) * p.vel0 +
	       (- 6 * tt + 6 * t    ) * p.pos1 / p.dt +
	       (  3 * tt - 2 * t    ) * p.vel1;
}

template<class T>
T _inter_angular_lerp_(const typename Interpolator<T>::Part &p, float t);

template<>
inline vector _inter_angular_lerp_(const Interpolator<vector>::Part &p, float t) {
	auto q0 = quaternion::rotation_v(p.pos0);
	auto q1 = quaternion::rotation_v(p.pos1);
	auto q = quaternion::interpolate(q0, q1, t);
	return q.get_angles();
}
template<>
inline float _inter_angular_lerp_(const Interpolator<float>::Part &p, float t)
{	return 0;	}
template<>
inline complex _inter_angular_lerp_(const Interpolator<complex>::Part &p, float t)
{	return complex(0,0);	}


template<class T>
int Interpolator<T>::canonize(float &t)
{
	t = clamp(t, 0.0f, t_sum * 0.99999f);
	foreachi(Part &p, part, i)
		if ((t >= p.t0) && (t <= p.t0 + p.dt)){
			t = (t - p.t0) / p.dt;
			return i;
		}
	return 0;
}

template<class T>
T Interpolator<T>::get(float t)
{
	if (!ready)
		update();
	if (part.num > 0){
		int index = canonize(t);
		if (type == TYPE_LERP)
			return _inter_lerp_<T>(part[index], t);
		if ((type == TYPE_CUBIC_SPLINE) or (type == TYPE_CUBIC_SPLINE_NOTANG))
			return _inter_cubic_spline_<T>(part[index], t);
		if (type == TYPE_ANGULAR_LERP)
			return _inter_angular_lerp_<T>(part[index], t);
		return part[index].pos0;
	}
	return _inter_zero_<T>();
}


template<class T>
T Interpolator<T>::getTang(float t)
{
	if (!ready)
		update();
	if (part.num > 0){
		int index = canonize(t);
		if (type == TYPE_LERP)
			return _inter_lerp_tang_<T>(part[index], t);
		if ((type == TYPE_CUBIC_SPLINE) or (type == TYPE_CUBIC_SPLINE_NOTANG))
			return _inter_cubic_spline_tang_<T>(part[index], t);
	}
	return _inter_zero_<T>();
}

template<>
inline void Interpolator<float>::print()
{
	if (!ready)
		update();
	msg_write("---");
	for (Part &p : part)
		msg_write(format("t0=%f dt=%f (%f  %f) -> (%f  %f)", p.t0, p.dt, p.pos0, p.vel0, p.pos1, p.vel1));
}

template<>
inline void Interpolator<vector>::print(){}

template<class T>
Array<T> Interpolator<T>::getList(Array<float> &t)
{
	//print();
	Array<T> r;
	r.resize(t.num);
	foreachi( float tt, t, i)
		r[i] = get(tt);
	return r;
}

template class Interpolator<float>;
template class Interpolator<vector>;
template class Interpolator<complex>;
//template class Interpolator<quaternion>;

