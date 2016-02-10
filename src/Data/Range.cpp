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
	num = r.num;
}

Range::Range()
{
	offset = 0;
	num = 0;
}

Range::Range(int _offset, int _length)
{
	offset = _offset;
	num = _length;
}

Range::~Range()
{
}

void Range::clear()
{
	offset = 0;
	num = 0;
}

string Range::str() const
{
	return format("(%d %d)", offset, num);
}

void Range::move(int dpos)
{
	offset += dpos;
}

void Range::resize(int new_length)
{
	num = new_length;
}

void Range::set_end(int end)
{
	num = end - offset;
}

void Range::set_start(int start)
{
	num = offset + num - start;
	offset = start;
}

void Range::invert()
{
	offset += num;
	num = - num;
}



int Range::start() const
{
	return offset;
}

int Range::length() const
{
	return num;
}

int Range::end() const
{
	return offset + num;
}

int Range::center() const
{
	return offset + num / 2;
}

bool Range::empty() const
{
	return num <= 0;
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
	return Range(offset + shift, num);
}

Range Range::operator- (int shift) const
{
	return Range(offset - shift, num);
}




