/*
 * Beat.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "Beat.h"


Beat::Beat(const Range &r, int _bar_no, int _beat_no, int _level)
{
	range = r;
	bar_no = _bar_no;
	beat_no = _beat_no;
	level = _level;
};

Range Beat::sub(int index, int parts)
{
	int length = range.length / parts;
	int start = range.offset + index * length;
	if (index == parts - 1)
		return Range(start, range.length - start);
	return Range(start, length);
}


