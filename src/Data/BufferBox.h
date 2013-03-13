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
	BufferBox(const BufferBox &b);
	virtual ~BufferBox();
	void operator=(const BufferBox &b);
	void __assign__(const BufferBox &other){	*this = other;	}

	int offset, num;
	Array<float> r, l;

	Array<string> peak;

	Range range();

	void clear();
	void resize(int length);
	bool is_ref();
	void make_own();
	void scale(float volume, float panning = 0);
	void swap_ref(BufferBox &b);
	void swap_value(BufferBox &b);
	void append(BufferBox &b);
	void set(const BufferBox &b, int offset, float volume);
	void add(const BufferBox &b, int offset, float volume, float panning);
	void set_16bit(const void *b, int offset, int length);
	void set_as_ref(const BufferBox &b, int offset, int length);
	void import(void *data, int channels, int bits, int samples);

	bool get_16bit_buffer(Array<short> &data);

	enum
	{
		PEAK_MODE_MAXIMUM,
		PEAK_MODE_SQUAREMEAN
	};

	void invalidate_peaks(const Range &r);
	void update_peaks(int mode);
};

#endif /* BUFFERBOX_H_ */
