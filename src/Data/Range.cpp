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

Range::~Range()
{
}

void Range::clear()
{
	offset = 0;
	length = 0;
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

void Range::invert()
{
	offset += length;
	length = - length;
}



int Range::get_offset() const
{
	return offset;
}

int Range::get_length() const
{
	return length;
}

int Range::get_end() const
{
	return offset + length;
}

bool Range::empty() const
{
	return length <= 0;
}

bool Range::overlaps(const Range &r) const
{
	if (empty() || r.empty())
		return false;
	return ((offset <= r.offset + r.length) && (offset + length >= r.offset));
}

bool Range::is_inside(int pos) const
{
	if (empty())
		return false;
	return ((pos >= offset) && (pos < offset + length));
}

Range Range::intersect(const Range &r) const
{
	int i0 = max(offset, r.get_offset());
	int i1 = min(offset + length, r.get_end());
	return Range(i0, i1 - i0);
}






