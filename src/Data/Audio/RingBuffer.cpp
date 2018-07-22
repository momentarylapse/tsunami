/*
 * RingBuffer.cpp
 *
 * thread-safe for single producer, single consumer
 *
 *  Created on: 26.01.2015
 *      Author: michi
 */

#include "RingBuffer.h"

RingBuffer::RingBuffer(int size)
{
	buf.resize(size);
	read_pos = 0;
	write_pos = 0;
	// if read_pos == write_pos
	//  -> considered empty!

	available_read = 0;
	available_write = size - 1;
}

RingBuffer::~RingBuffer()
{
}

void RingBuffer::clear()
{
	std::unique_lock<std::shared_timed_mutex> lock(buf.mtx);

	write_pos = 0;
	read_pos = 0;

	available_read = 0;
	available_write = buf.length - 1;
}

// how many samples are readable?
int RingBuffer::available()
{
	return available_read;
	/*if (write_pos < read_pos)
		return write_pos - read_pos + buf.length;
	return write_pos - read_pos;*/
}

// how many samples are writable?
//   why -1?
//      - if we write until read_pos=write_pos -> empty again!
int RingBuffer::writable_size()
{
	return available_write;
	/*if (read_pos <= write_pos)
		return read_pos - write_pos + buf.length - 1;
	return read_pos - write_pos - 1;*/
}

void RingBuffer::_move_read_pos(int delta)
{
	read_pos += delta;
	if (read_pos >= buf.length)
		read_pos -= buf.length;
	available_read -= delta;
	available_write += delta;
}
void RingBuffer::_move_write_pos(int delta)
{
	write_pos += delta;
	if (write_pos >= buf.length)
		write_pos -= buf.length;
	available_read += delta;
	available_write -= delta;
}

int RingBuffer::read(AudioBuffer& b)
{
	std::unique_lock<std::shared_timed_mutex> lock(buf.mtx);

	int samples = min(b.length, available());

	int samples_a = min(samples, buf.length - read_pos);
	b.set_x(buf, -read_pos, read_pos + samples_a, 1.0f);
	_move_read_pos(samples_a);

	int samples_b = samples - samples_a;
	if (samples_b == 0)
		return samples_a;

	AudioBuffer bb;
	bb.set_as_ref(b, samples_a,  samples_b);
	bb.set_x(buf, 0, samples_b, 1.0f);
	_move_read_pos(samples_b);
	return samples;
}

void RingBuffer::write(AudioBuffer& b)
{
	std::unique_lock<std::shared_timed_mutex> lock(buf.mtx);

	int samples = min(b.length, writable_size());

	int size_a = min(samples, buf.length - write_pos);
	buf.set(b, write_pos, 1.0f);

	int size_b = samples - size_a;
	if (size_b > 0){
		buf.set(b, -size_a, 1.0f);
	}

	_move_write_pos(samples);
}

// size > 0: from read_pos
// size < 0: from write_pos backwards
void RingBuffer::peek_ref(AudioBuffer &b, int size)
{
	if (size >= 0){
		size = min(size, available());
		size = min(size, buf.length - read_pos);
		b.set_as_ref(buf, read_pos, size);
	}else{
		size = min(-size, write_pos.load());
		b.set_as_ref(buf, write_pos - size, size);
	}
}

// get a reference of the internal buffer for reading (later)
void RingBuffer::read_ref(AudioBuffer &b, int size)
{
	peek_ref(b, size);
}

void RingBuffer::read_ref_done(AudioBuffer &b)
{
	_move_read_pos(b.length);
}

// get a reference of the internal buffer to write into (later)
void RingBuffer::write_ref(AudioBuffer &b, int size)
{
	size = min(size, buf.length - write_pos);
	b.set_as_ref(buf, write_pos, size);
}

void RingBuffer::write_ref_done(AudioBuffer &b)
{
	_move_write_pos(b.length);
}
