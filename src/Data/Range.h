/*
 * Range.h
 *
 *  Created on: 11.04.2012
 *      Author: michi
 */

#ifndef RANGE_H_
#define RANGE_H_

class Range
{
public:
	Range();
	Range(int _offset, int _length);
	Range(const Range &r);
	virtual ~Range();

	void clear();

	void resize(int new_length);
	void move(int dpos);
	void set_end(int end);

	void invert();

	int offset, length;
	int get_offset() const;
	int get_length() const;
	int get_end() const;
	bool empty() const;

	bool is_inside(int pos) const;
	bool overlaps(const Range &r) const;
};

#endif /* RANGE_H_ */
