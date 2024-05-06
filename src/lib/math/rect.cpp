#include "rect.h"
#include "vec2.h"

const rect rect::ID = rect(0,1, 0,1);
const rect rect::ID_SYM = rect(-1,1, -1,1);
const rect rect::EMPTY = rect(0,0, 0,0);

rect::rect(float x1, float x2, float y1, float y2) {
	this->x1 = x1;
	this->x2 = x2;
	this->y1 = y1;
	this->y2 = y2;
}

string rect::str() const {
	return format("(%f:%f, %f:%f)", x1, x2, y1, y2);
}

float rect::width() const {
	return x2 - x1;
}

float rect::height() const {
	return y2 - y1;
}

vec2 rect::center() const {
	return {(x1 + x2) / 2, (y1 + y2) / 2};
}

vec2 rect::size() const {
	return {width(), height()};
}

float rect::area() const {
	return (x2 - x1) * (y2 - y1);
}

bool rect::inside(const vec2 &p) const {
	return (p.x >= x1) and (p.x <= x2) and (p.y >= y1) and (p.y <= y2);
}

// r in this?
bool rect::covers(const rect &r) const {
	return (r.x1 >= x1) and (r.x2 <= x2) and (r.y1 >= y1) and (r.y2 <= y2);
}

bool rect::overlaps(const rect &r) const {
	if (covers(r) or r.covers(*this))
		return true;
	if (inside({r.x1, r.y1}))
		return true;
	if (inside({r.x2, r.y1}))
		return true;
	if (inside({r.x1, r.y2}))
		return true;
	if (inside({r.x2, r.y2}))
		return true;
	return false;
}

rect rect::grow(float d) const {
	return rect(x1 - d, x2 + d, y1 - d, y2 + d);
}

bool rect::operator==(const rect &r) const {
	return (x1 == r.x1) and (y1 == r.y1) and (x2 == r.x2) and (y2 == r.y2);
}

bool rect::operator!=(const rect &r) const {
	return !(*this == r);
}

// assume a1 <= a2, b1 <= b2
void range_intersect(float a1, float a2, float b1, float b2, float &o1, float &o2) {
	o1 = max(a1, b1);
	o2 = min(a2, b2);
	//o2 = max(o1, o2);
}

// intersection
rect rect::operator&&(const rect &r) const {
	rect o = r;
	range_intersect(x1, x2, r.x1, r.x2, o.x1, o.x2);
	range_intersect(y1, y2, r.y1, r.y2, o.y1, o.y2);
	return o;
}

// hull
rect rect::operator||(const rect &r) const {
	rect o;
	o.x1 = min(x1, r.x1);
	o.y1 = min(y1, r.y1);
	o.x2 = max(x2, r.x2);
	o.y2 = max(y2, r.y2);
	return o;
}
