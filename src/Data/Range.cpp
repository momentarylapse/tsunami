/*
 * Range.cpp
 *
 *  Created on: 11.04.2012
 *      Author: michi
 */

#include "Range.h"

Range::Range(const Range & r)
{
	offset = r.offset;
	length = r.length;
}

Range::Range(int _offset, int _length)
{
	offset = _offset;
	length = _length;
}

Range::~Range()
{
}

void Range::move(int dpos)
{
	offset += dpos;
}

void Range::resize(int new_length)
{
	length = new_length;
}



int Range::get_offset()
{
	return offset;
}

int Range::get_length()
{
	return length;
}

int Range::get_end()
{
	return offset + length;
}




