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
	Range(int _offset, int _length);
	Range(const Range &r);
	virtual ~Range();

	void resize(int new_length);
	void move(int dpos);

	int offset, length;
	int get_offset();
	int get_length();
	int get_end();
};

#endif /* RANGE_H_ */
