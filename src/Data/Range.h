/*
 * Range.h
 *
 *  Created on: 11.04.2012
 *      Author: michi
 */

#ifndef RANGE_H_
#define RANGE_H_

class string;

class Range
{
public:
	Range();
	Range(int _offset, int _length);
	Range(const Range &r);
	~Range();

	string str() const;

	void clear();

	void resize(int new_length);
	void move(int dpos);
	void set_start(int start);
	void set_end(int end);

	void invert();

	int offset, num;
	int start() const;
	int length() const;
	int end() const;
	bool empty() const;

	bool is_inside(int pos) const;
	bool overlaps(const Range &r) const;
	bool covers(const Range &r) const;
	Range intersect(const Range &r) const;

	Range operator||(const Range &r) const;
	Range operator&&(const Range &r) const;
	Range operator+ (int shift) const;
};

#endif /* RANGE_H_ */
