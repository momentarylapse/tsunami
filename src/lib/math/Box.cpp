#include "Box.h"


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

vec3 Box::to_absolute(const vec3& p) const {
	const vec3 s = size();
	return min + vec3(p.x * s.x, p.y * s.y, p.z * s.z);
}

vec3 Box::to_relative(const vec3& p) const {
	const vec3 s = size();
	const vec3 q = p - min;
	return vec3(q.x / s.x, q.y / s.y, q.z / s.z);
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



