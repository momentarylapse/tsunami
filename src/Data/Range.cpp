/*
 * Range.cpp
 *
 *  Created on: 11.04.2012
 *      Author: michi
 */

#include "Range.h"
#include "../lib/file/file.h"

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

bool Range::empty() const
{
	return num <= 0;
}

// do <this> and <r> overlap?
bool Range::overlaps(const Range &r) const
{
	if (empty() || r.empty())
		return false;
	return ((start() <= r.end()) && (end() >= r.start()));
}

// does <this> cover <r>?
bool Range::covers(const Range &r) const
{
	if (empty() || r.empty())
		return false;
	return ((start() <= r.start()) && (end() >= r.end()));
}

bool Range::is_inside(int pos) const
{
	if (empty())
		return false;
	return ((pos >= start()) && (pos < end()));
}

Range Range::intersect(const Range &r) const
{
	int i0 = max(start(), r.start());
	int i1 = min(end(), r.end());
	return Range(i0, i1 - i0);
}






