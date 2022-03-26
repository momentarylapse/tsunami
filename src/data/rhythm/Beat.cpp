/*
 * Beat.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "Beat.h"


Beat::Beat(const Range &r, int _beat_no, int _level, int _bar_index, int _bar_no) {
	range = r;
	bar_index = _bar_index;
	bar_no = _bar_no;
	beat_no = _beat_no;
	level = _level;
}

Range Beat::sub(int index, int parts) {
	int length = range.length / parts;
	int start = range.offset + index * length;
	if (index == parts - 1)
		return Range(start, range.length - start);
	return Range(start, length);
}


