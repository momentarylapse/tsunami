#include "Box.h"


const Box Box::EMPTY{{0,0,0}, {0,0,0}};
const Box Box::ID{{0,0,0}, {1,1,1}};
const Box Box::ID_SYM{{-1,-1,-1}, {1,1,1}};

vec3 Box::center() const {
	return (min + max) / 2;
}

vec3 Box::size() const {
	return max - min;
}

Box Box::canonical() const {
	Box r = *this;
	r.min._min(max);
	r.max._max(min);
	return r;
}

bool Box::is_inside(const vec3& p) const {
	if (p.x < min.x || p.y < min.y || p.z < min.z)
		return false;
	if (p.x > max.x || p.y > max.y || p.z > max.z)
		return false;
	return true;
	//return p.between(min, max); // WTF is that implementation?!?
}

vec3 Box::to_absolute(const vec3& p) const {
	const vec3 s = size();
	return min + vec3(p.x * s.x, p.y * s.y, p.z * s.z);
}

vec3 Box::to_relative(const vec3& p) const {
	const vec3 s = size();
	const vec3 q = p - min;
	return vec3(q.x / s.x, q.y / s.y, q.z / s.z);
}

string Box::str() const {
	return ::str(min) + ":" + ::str(max);
}

bool Box::operator==(const Box &o) const {
	return min == o.min && max == o.max;
}

bool Box::operator!=(const Box &o) const {
	return !(*this == o);
}

Box Box::operator||(const Box& b) const {
	Box r = *this;
	r.min._min(b.min);
	r.max._max(b.max);
	return r;
}

Box Box::operator&&(const Box& b) const {
	Box r = *this;
	r.min._max(b.min);
	r.max._min(b.max);
	return r;
}



