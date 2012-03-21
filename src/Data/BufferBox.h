/*
 * BufferBox.h
 *
 *  Created on: 21.03.2012
 *      Author: michi
 */

#ifndef BUFFERBOX_H_
#define BUFFERBOX_H_

#include "../lib/file/file.h"

class BufferBox
{
public:
	BufferBox();
	virtual ~BufferBox();

	int offset, num;
	Array<float> r, l;

	void clear();
	void resize(int length);
	bool is_ref();
	void make_own();
	void scale(float volume);
	void swap(BufferBox &b);
	void set(const BufferBox &b, int offset, float volume);
	void add(const BufferBox &b, int offset, float volume);
	void set_16bit(const void *b, int offset, int length);
	void set_as_ref(const BufferBox &b, int offset, int length);
};

#endif /* BUFFERBOX_H_ */
