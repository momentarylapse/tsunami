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

	void readRef(BufferBox &b, int size);
	void peekRef(BufferBox &b, int size);
	void writeRef(BufferBox &b, int size);

	void moveReadPos(int delta);
	void moveWritePos(int delta);

	void clear();

	BufferBox buf;
	int read_pos;
	int write_pos;
};

#endif /* SRC_DATA_RINGBUFFER_H_ */
