/*
 * Range.cpp
 *
 *  Created on: 11.04.2012
 *      Author: michi
 */

#include "Range.h"
#include "../lib/file/file.h"

const int Range::BEGIN = -1000000000; // just less than 0x4000.0000, so that also length < 0x8000.000 (staying positive)
const int Range::END = 1000000000;
const Range Range::ALL = RangeTo(BEGIN, END);
const Range Range::NONE = Range(0, 0);


Range RangeTo(int start, int end) {
	return Range(start, end - start);
}

Range::Range(const Range & r) {
	offset = r.offset;
	length = r.length;
}

Range::Range() {
	offset = 0;
	length = 0;
}

Range::Range(int _offset, int _length) {
	offset = _offset;
	length = _length;
}

void Range::clear() {
	offset = 0;
	length = 0;
}

string Range::str() const {
	return format("(%d:%d)", start(), end());
}

void Range::move(int dpos) {
	offset += dpos;
}

void Range::resize(int new_length) {
	length = new_length;
}

void Range::set_end(int end) {
	length = end - offset;
}

void Range::set_start(int start) {
	length = offset + length - start;
	offset = start;
}

void Range::invert() {
	offset += length;
	length = - length;
}

Range Range::canonical() const {
	auto r = *this;
	if (r.length < 0)
		r.invert();
	return r;
}



int Range::start() const {
	return offset;
}

int Range::end() const {
	return offset + length;
}

int Range::center() const {
	return offset + length / 2;
}

bool Range::is_empty() const {
	return length == 0;
}

bool Range::is_none() const {
	return length == 0 and offset == 0;
}

// do <this> and <r> have at least one sample of overlap?
bool Range::overlaps(const Range &r) const {
	return ((start() < r.end()) and (end() > r.start()));
}

// does <this> completely cover <r>?
bool Range::covers(const Range &r) const {
	return ((start() <= r.start()) and (end() >= r.end()));
}

bool Range::is_inside(int pos) const {
	return ((pos >= start()) and (pos < end()));
}

bool Range::is_more_inside(int pos) const {
	return ((pos > start()) and (pos < end() - 1));
}

Range Range::intersect(const Range &r) const {
	if (is_none() or r.is_none())
		return NONE;
	int i0 = max(start(), r.start());
	int i1 = min(end(), r.end());
	return RangeTo(i0, i1);
}

Range Range::operator||(const Range &r) const {
	if (is_none())
		return r;
	if (r.is_none())
		return *this;
	int i0 = min(start(), r.start());
	int i1 = max(end(), r.end());
	return RangeTo(i0, i1);
}

Range Range::operator&&(const Range &r) const {
	return intersect(r);
}

Range Range::operator+ (int shift) const {
	return Range(offset + shift, length);
}

Range Range::operator- (int shift) const {
	return Range(offset - shift, length);
}

bool Range::operator==(const Range &r) const {
	return (offset == r.offset) and (length == r.length);
}

bool Range::operator!=(const Range &r) const {
	return !(*this == r);
}


