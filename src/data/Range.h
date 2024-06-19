/*
 * Range.h
 *
 *  Created on: 11.04.2012
 *      Author: michi
 */

#ifndef RANGE_H_
#define RANGE_H_

#include "../lib/base/base.h"

class string;

namespace tsunami {

class Range {
public:
	Range();
	Range(int _offset, int _length);
	Range(const Range &r);

	static const Range ALL;
	static const Range NONE;
	
	static const int BEGIN;
	static const int END;

	string str() const;

	void clear();

	void resize(int new_length);
	void move(int dpos);
	void set_start(int start);
	void set_end(int end);

	void invert();
	Range canonical() const;

	int offset, length;
	int start() const;
	int end() const;
	int center() const;
	bool is_empty() const;
	bool is_none() const;

	bool is_inside(int pos) const;
	bool is_more_inside(int pos) const;
	bool overlaps(const Range &r) const;
	bool covers(const Range &r) const;
	Range intersect(const Range &r) const;

	Range operator||(const Range &r) const;
	Range operator&&(const Range &r) const;
	Range operator+ (int shift) const;
	Range operator- (int shift) const;
	
	bool operator==(const Range &r) const;
	bool operator!=(const Range &r) const;

	static Range to(int start, int end);
	Range scale_rel(const Range &from, const Range &to) const;
};

}

#endif /* RANGE_H_ */
