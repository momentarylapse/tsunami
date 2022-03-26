/*
 * RingBuffer.h
 *
 *  Created on: 26.01.2015
 *      Author: michi
 */

#ifndef SRC_DATA_AUDIO_RINGBUFFER_H_
#define SRC_DATA_AUDIO_RINGBUFFER_H_

#include "AudioBuffer.h"
#include <atomic>

class RingBuffer {
public:
	RingBuffer(int size);
	~RingBuffer();

	void _cdecl __init__(int size);
	void _cdecl __delete__();

	// internal
	void _move_read_pos(int delta);
	void _move_write_pos(int delta);

	int available();
	int writable_size();

	int read(AudioBuffer &b);
	int write(const AudioBuffer &b);

	enum class PeekMode {
		FORWARD_REF,
		FORWARD_COPY_WRAP,
		BACKWARD_REF,
		BACKWARD_COPY_WRAP,
	};

	void read_ref(AudioBuffer &b, int size);
	void read_ref_done(AudioBuffer &b);
	void read_ref_cancel(AudioBuffer &b);
	void peek(AudioBuffer &b, int size, PeekMode mode);
	void write_ref(AudioBuffer &b, int size);
	void write_ref_done(AudioBuffer &b);
	void write_ref_cancel(AudioBuffer &b);

	void clear();
	void set_channels(int channels);

	AudioBuffer buf;
	std::atomic<int> read_pos;
	std::atomic<int> write_pos;
	std::atomic<int> available_read;
	std::atomic<int> available_write;
};

#endif /* SRC_DATA_AUDIO_RINGBUFFER_H_ */
