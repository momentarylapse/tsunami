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

// do <this> and <r> overlap?
bool Range::overlaps(const Range &r) const
{
	return ((start() <= r.end()) and (end() >= r.start()));
}

// does <this> cover <r>?
bool Range::covers(const Range &r) const
{
	return ((start() <= r.start()) and (end() >= r.end()));
}

bool Range::is_inside(int pos) const
{
	return ((pos >= start()) and (pos < end()));
}

Range Range::intersect(const Range &r) const
{
	int i0 = max(start(), r.start());
	int i1 = min(end(), r.end());
	return Range(i0, i1 - i0);
}

Range Range::operator||(const Range &r) const
{
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




