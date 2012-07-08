/*
 * BufferBox.h
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#ifndef BUFFERBOX_H_
#define BUFFERBOX_H_

#include "../lib/file/file.h"
#include "Range.h"

class BufferBox
{
public:
	BufferBox();
	virtual ~BufferBox();
	void __assign__(const BufferBox &other){	*this = other;	}

	int offset, num;
	Array<float> r, l;

	Range range();

	void clear();
	void resize(int length);
	bool is_ref();
	void make_own();
	void scale(float volume);
	void swap_ref(BufferBox &b);
	void swap_value(BufferBox &b);
	void set(const BufferBox &b, int offset, float volume);
	void add(const BufferBox &b, int offset, float volume);
	void set_16bit(const void *b, int offset, int length);
	void set_as_ref(const BufferBox &b, int offset, int length);
	void import(void *data, int channels, int bits, int samples);

	void get_16bit_buffer(Array<short> &data);
};

#endif /* BUFFERBOX_H_ */
