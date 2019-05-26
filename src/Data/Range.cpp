/*
 * Range.cpp
 *
 *  Created on: 11.04.2012
 *      Author: michi
 */

#include "Range.h"
#include "../lib/file/file.h"

const Range Range::ALL = Range(-0x4000000, 0x8000000); // TODO
const Range Range::EMPTY = Range(0, 0);


Range RangeTo(int start, int end)
{
	return Range(start, end - start);
}

Range::Range(const Range & r)
{
	offset = r.offset;
	length = r.length;
}

Range::Range()
{
	offset = 0;
	length = 0;
}

Range::Range(int _offset, int _length)
{
	offset = _offset;
	length = _length;
}

void Range::clear()
{
	offset = 0;
	length = 0;
}

string Range::str() const
{
	return format("(%d %d)", offset, length);
}

void Range::move(int dpos)
{
	offset += dpos;
}

void Range::resize(int new_length)
{
	length = new_length;
}

void Range::set_end(int end)
{
	length = end - offset;
}

void Range::set_start(int start)
{
	length = offset + length - start;
	offset = start;
}

void Range::invert()
{
	offset += length;
	length = - length;
}

Range Range::canonical() const
{
	auto r = *this;
	if (r.length < 0)
		r.invert();
	return r;
}



int Range::start() const
{
	return offset;
}

int Range::end() const
{
	return offset + length;
}

int Range::center() const
{
	return offset + length / 2;
}

bool Range::empty() const
{
	return length <= 0;
}

// do <this> and <r> have at least one sample of overlap?
bool Range::overlaps(const Range &r) const
{
	return ((start() < r.end()) and (end() > r.start()));
}

// does <this> completely cover <r>?
bool Range::covers(const Range &r) const
{
	return ((start() <= r.start()) and (end() >= r.end()));
}

bool Range::is_inside(int pos) const
{
	return ((pos >= start()) and (pos < end()));
}

bool Range::is_more_inside(int pos) const
{
	return ((pos > start()) and (pos < end() - 1));
}

Range Range::intersect(const Range &r) const
{
	if (empty() or r.empty())
		return EMPTY;
	int i0 = max(start(), r.start());
	int i1 = min(end(), r.end());
	return Range(i0, i1 - i0);
}

Range Range::operator||(const Range &r) const
{
	if (empty())
		return r;
	if (r.empty())
		return *this;
	int i0 = min(start(), r.start());
	int i1 = max(end(), r.end());
	return Range(i0, i1 - i0);
}

Range Range::operator&&(const Range &r) const
{
	return intersect(r);
}

Range Range::operator+ (int shift) const
{
	return Range(offset + shift, length);
}

Range Range::operator- (int shift) const
{
	return Range(offset - shift, length);
}




