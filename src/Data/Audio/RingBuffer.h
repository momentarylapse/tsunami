/*
 * RingBuffer.h
 *
 *  Created on: 26.01.2015
 *      Author: michi
 */

#ifndef SRC_DATA_AUDIO_RINGBUFFER_H_
#define SRC_DATA_AUDIO_RINGBUFFER_H_

#include "AudioBuffer.h"

class RingBuffer {
public:
	RingBuffer(int size);
	virtual ~RingBuffer();

	int available();
	int writable_size();
	int read(AudioBuffer &b);
	void write(AudioBuffer &b);

	void readRef(AudioBuffer &b, int size);
	void peekRef(AudioBuffer &b, int size);
	void writeRef(AudioBuffer &b, int size);

	void moveReadPos(int delta);
	void moveWritePos(int delta);

	void clear();

	AudioBuffer buf;
	int read_pos;
	int write_pos;
};

#endif /* SRC_DATA_AUDIO_RINGBUFFER_H_ */
