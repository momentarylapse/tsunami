#include "math.h"

const rect rect::ID = rect(0,1,0,1);
const rect rect::EMPTY = rect(0,0,0,0);

rect::rect(float x1, float x2, float y1, float y2)
{
	this->x1 = x1;
	this->x2 = x2;
	this->y1 = y1;
	this->y2 = y2;
}

string rect::str() const
{
	return format("(%f:%f, %f:%f)", x1, x2, y1, y2);
}

float rect::width() const
{
	return x2 - x1;
}

float rect::height() const
{
	return y2 - y1;
}

float rect::mx() const
{
	return (x1 + x2) / 2;
}

float rect::my() const
{
	return (y1 + y2) / 2;
}

float rect::area() const
{
	return (x2 - x1) * (y2 - y1);
}

bool rect::inside(float x, float y) const
{
	return (x >= x1) and (x <= x2) and (y >= y1) and (y <= y2);
}

// r in this?
bool rect::covers(const rect &r) const
{
	return (r.x1 >= x1) and (r.x2 <= x2) and (r.y1 >= y1) and (r.y2 <= y2);
}

bool rect::overlaps(const rect &r) const
{
	if (covers(r) or r.covers(*this))
		return true;
	if (inside(r.x1, r.y1))
		return true;
	if (inside(r.x2, r.y1))
		return true;
	if (inside(r.x1, r.y2))
		return true;
	if (inside(r.x2, r.y2))
		return true;
	return false;
}

bool rect::operator ==(const rect &r) const
{
	return (x1 == r.x1) and (y1 == r.y1) and (x2 == r.x2) and (y2 == r.y2);
}

bool rect::operator !=(const rect &r) const
{
	return !(*this == r);
}
