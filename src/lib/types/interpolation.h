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



template<class T>
Interpolator<T>::Interpolator(Type _type)
{
	type = _type;
	clear();
}

template<class T>
void Interpolator<T>::__init__()
{
	type = TYPE_LERP;
	part.__init__();
	clear();
}

template<class T>
void Interpolator<T>::set_type(const string &_type)
{
	if (_type == "lerp")
		type = TYPE_LERP;
	else if (_type == "cubic-spline")
		type = TYPE_CUBIC_SPLINE;
	else if (_type == "cubic-spline-notang")
		type = TYPE_CUBIC_SPLINE_NOTANG;
	else if (_type == "angular-lerp")
		type = TYPE_ANGULAR_LERP;
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
}



template<class T>
T _inter_zero_();

template<>
inline float _inter_zero_<float>()
{	return 0;	}

template<>
inline vector _inter_zero_<vector>()
{	return v0;	}



template<class T>
void Interpolator<T>::add(const T &p, float dt)
{	add2(p, _inter_zero_<T>(), dt);	}


template<class T>
void Interpolator<T>::add2(const T &p, const T &v, float dt)
{
	if (!empty){
		temp.pos1 = p;
		temp.vel1 = v;
		temp.t0 = t_sum;
		temp.dt = dt;
		part.add(temp);
		t_sum += temp.dt;
	}
	temp.pos0 = p;
	temp.vel0 = v;
	temp.dt = 0;
	empty = false;
}


template<class T>
void Interpolator<T>::jump(const T &p, const T &v)
{
	temp.pos0 = p;
	temp.vel0 = v;
	empty = false;
}


template<class T>
void Interpolator<T>::update()
{
	if (type == TYPE_CUBIC_SPLINE_NOTANG){
		type = TYPE_CUBIC_SPLINE;
		part[0].vel0 = (part[0].pos1 - part[0].pos0) / part[0].dt;
		part.back().vel1 = (part.back().pos1 - part.back().pos0) / part.back().dt;
		for (int i=1;i<part.num;i++){
			T v = (part[i].pos1 - part[i - 1].pos0) / (part[i - 1].dt + part[i].dt);
			part[i - 1].vel1 = v;
			part[i    ].vel0 = v;
		};
	}
	ready = true;
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
	float tt = t * t;
	float ttt = tt * t;
	return (  2 * ttt - 3 * tt + 1) * p.pos0 +
	       (      ttt - 2 * tt + t) * p.vel0 * p.dt +
	       (- 2 * ttt + 3 * tt    ) * p.pos1 +
	       (      ttt -     tt    ) * p.vel1 * p.dt;
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
inline vector _inter_angular_lerp_(const Interpolator<vector>::Part &p, float t)
{
	quaternion q0, q1, q;
	QuaternionRotationV(q0, p.pos0);
	QuaternionRotationV(q1, p.pos1);
	QuaternionInterpolate(q, q0, q1, t);
	return QuaternionToAngle(q);
}
template<>
inline float _inter_angular_lerp_(const Interpolator<float>::Part &p, float t)
{	return 0;	}

float clampf(float, float, float);


template<class T>
int Interpolator<T>::canonize(float &t)
{
	t = clampf(t, 0, 0.99999f) * t_sum;
	foreachi(part, p, i)
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
		if (type == TYPE_CUBIC_SPLINE)
			return _inter_cubic_spline_<T>(part[index], t);
		if (type == TYPE_ANGULAR_LERP)
			return _inter_angular_lerp_<T>(part[index], t);
		return part[index].pos0;
	}
	return _inter_zero_<T>();
}


template<class T>
T Interpolator<T>::get_tang(float t)
{
	if (!ready)
		update();
	if (part.num > 0){
		int index = canonize(t);
		if (type == TYPE_LERP)
			return _inter_lerp_tang_<T>(part[index], t);
		if (type == TYPE_CUBIC_SPLINE)
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
	foreach(part, p)
		msg_write(format("t0=%f dt=%f (%f  %f) -> (%f  %f)", p.t0, p.dt, p.pos0, p.vel0, p.pos1, p.vel1));
}

template<>
inline void Interpolator<vector>::print(){}

template<class T>
Array<T> Interpolator<T>::get_list(Array<float> &t)
{
	//print();
	Array<T> r;
	r.resize(t.num);
	foreachi(t, tt, i)
		r[i] = get(tt);
	return r;
}



#endif /* INTERPOLATION_H_ */
