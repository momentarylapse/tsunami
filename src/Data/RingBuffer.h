/*
 * RingBuffer.h
 *
 *  Created on: 26.01.2015
 *      Author: michi
 */

#ifndef SRC_DATA_RINGBUFFER_H_
#define SRC_DATA_RINGBUFFER_H_

#include "BufferBox.h"

class RingBuffer {
public:
	RingBuffer(int size);
	virtual ~RingBuffer();

	int available();
	void read(BufferBox &b);
	void write(BufferBox &b);

	BufferBox buf;
	int read_pos;
	int write_pos;
};

#endif /* SRC_DATA_RINGBUFFER_H_ */
