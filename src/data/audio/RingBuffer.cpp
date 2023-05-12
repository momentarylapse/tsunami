/*
 * RingBuffer.cpp
 *
 * thread-safe for single producer, single consumer
 *
 *  Created on: 26.01.2015
 *      Author: michi
 */

#include "RingBuffer.h"
#include <mutex>
#include <cstdio>

RingBuffer::RingBuffer(int size) {
	buf.resize(size);
	read_pos = 0;
	write_pos = 0;
	// if read_pos == write_pos
	//  -> considered empty!

	available_read = 0;
	available_write = size - 1;
}

RingBuffer::~RingBuffer() {
}

void RingBuffer::__init__(int size) {
	new(this) RingBuffer(size);
}

void RingBuffer::__delete__() {
	this->RingBuffer::~RingBuffer();
}

void RingBuffer::clear() {
	std::unique_lock<std::shared_timed_mutex> lock(buf.mtx);

	write_pos = 0;
	read_pos = 0;

	available_read = 0;
	available_write = buf.length - 1;
}

void RingBuffer::set_channels(int channels) {
	std::unique_lock<std::shared_timed_mutex> lock(buf.mtx);
	buf.set_channels(channels);
}

// how many samples are readable?
int RingBuffer::available() {
	return available_read;
	/*if (write_pos < read_pos)
		return write_pos - read_pos + buf.length;
	return write_pos - read_pos;*/
}

// how many samples are writable?
//   why -1?
//      - if we write until read_pos=write_pos -> empty again!
int RingBuffer::writable_size() {
	return available_write;
	/*if (read_pos <= write_pos)
		return read_pos - write_pos + buf.length - 1;
	return read_pos - write_pos - 1;*/
}

// use with care!
void RingBuffer::_move_read_pos(int delta) {
	read_pos += delta;
	if (read_pos >= buf.length)
		read_pos -= buf.length;
	available_read -= delta;
	available_write += delta;
}

// use with care!
void RingBuffer::_move_write_pos(int delta) {
	write_pos += delta;
	if (write_pos >= buf.length)
		write_pos -= buf.length;
	available_read += delta;
	available_write -= delta;
}

int RingBuffer::read(AudioBuffer& b) {
	AudioBuffer ref;
	read_ref(ref, b.length);
	int samples_a = ref.length;
	b.set(ref, 0, 1.0f);
	read_ref_done(ref);

	int samples_b = b.length - samples_a;
	if (samples_b <= 0)
		return samples_a;

	read_ref(ref, samples_b);
	b.set(ref, samples_a, 1.0f);
	read_ref_done(ref);
	return samples_a + ref.length;
}

int RingBuffer::write(const AudioBuffer& b) {
	AudioBuffer ref;
	write_ref(ref, b.length);
	int samples_a = ref.length;
	ref.set(b, 0, 1.0f);
	write_ref_done(ref);

	int samples_b = b.length - samples_a;
	if (samples_b == 0)
		return samples_a;

	write_ref(ref, samples_b);
	ref.set(((AudioBuffer&)b).ref(samples_a, b.length), 0);
	write_ref_done(ref);
	return samples_a + ref.length;

	/*int samples_b = b.length - samples_a;
	if (samples_b == 0)
		return samples_a;

	write_ref(ref, samples_b);
	ref.set_x(b, samples_a, ref.length, 1.0f);
	write_ref_done(ref);
	return samples_a + samples_b;*/
}

// size > 0: from read_pos
// size < 0: from write_pos backwards
void RingBuffer::peek(AudioBuffer &b, int size, PeekMode mode) {
	if (mode == PeekMode::FORWARD_REF) {
		size = min(size, available());
		size = min(size, buf.length - read_pos);
		b.set_as_ref(buf, read_pos, size);
	} else if (mode == PeekMode::FORWARD_COPY_WRAP) {
		// TODO implement
		printf("RingBuffer.peek(FORWARD_COPY_WRAP...\n");
	} else if (mode == PeekMode::BACKWARD_REF) {
		size = min(-size, write_pos.load());
		b.set_as_ref(buf, write_pos - size, size);
	} else if (mode == PeekMode::BACKWARD_COPY_WRAP) {
		b.resize(size);
		int wp = write_pos.load();
		int target_offset = 0;
		int source_offset_end = wp; // end of range!
		while (size > 0) {
			int chunk = min(size, source_offset_end);
			b.set_x(buf, source_offset_end - chunk, chunk, target_offset, 1.0f);
			size -= chunk;
			target_offset += chunk;
			source_offset_end = buf.length;
		}
	}
}

// get a reference of the internal buffer for reading (later)
void RingBuffer::read_ref(AudioBuffer &b, int size) {
	peek(b, size, PeekMode::FORWARD_REF);
}

void RingBuffer::read_ref_done(AudioBuffer &b) {
	_move_read_pos(b.length);
}

// get a reference of the internal buffer to write into (later)
void RingBuffer::write_ref(AudioBuffer &b, int size) {
	// directly writable size?
	if (read_pos == 0) {
		// if we write to the end, we jump to write_pos=0
		// ...and that would mean, we're empty...
		size = min(size, buf.length - write_pos - 1);
	} else {
		size = min(size, buf.length - write_pos);
		if (read_pos > write_pos)
			size = min(size, read_pos - write_pos);
	}
	b.set_as_ref(buf, write_pos, size);
}

void RingBuffer::write_ref_done(AudioBuffer &b) {
	_move_write_pos(b.length);
}

// not needed... just to make code more readable
void RingBuffer::write_ref_cancel(AudioBuffer &b){}
void RingBuffer::read_ref_cancel(AudioBuffer &b){}
