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
	if (p.x < min.x or p.y < min.y or p.z < min.z)
		return false;
	if (p.x > max.x or p.y > max.y or p.z > max.z)
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
	return {q.x / s.x, q.y / s.y, q.z / s.z};
}

vec3 Box::clamp(const vec3 &p) const {
	return {::clamp(p.x, min.x, max.x), ::clamp(p.y, min.y, max.y), ::clamp(p.z, min.z, max.z)};
}

vec3 Box::loop(const vec3 &p) const {
	return {::loop(p.x, min.x, max.x), ::loop(p.y, min.y, max.y), ::loop(p.z, min.z, max.z)};
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

Box Box::operator+(const vec3& d) const {
	return {min + d, max + d};
}

Box Box::operator-(const vec3& d) const {
	return {min - d, max - d};
}

Box Box::operator*(float scale) const {
	return {min * scale, max * scale};
}

void Box::operator+=(const vec3 &d) {
	min += d;
	max += d;
}

void Box::operator-=(const vec3 &d) {
	min -= d;
	max -= d;
}

void Box::operator*=(float scale) {
	min *= scale;
	max *= scale;
}

