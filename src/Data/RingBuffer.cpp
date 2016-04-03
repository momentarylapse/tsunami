/*
 * RingBuffer.cpp
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
}

RingBuffer::~RingBuffer()
{
}

void RingBuffer::clear()
{
	write_pos = 0;
	read_pos = 0;
}

int RingBuffer::available()
{
	if (write_pos < read_pos)
		return write_pos - read_pos + buf.length;
	return write_pos - read_pos;
}

void RingBuffer::moveReadPos(int delta)
{
	read_pos += delta;
	if (read_pos >= buf.length)
		read_pos -= buf.length;
}
void RingBuffer::moveWritePos(int delta)
{
	write_pos += delta;
	if (write_pos >= buf.length)
		write_pos -= buf.length;
}

void RingBuffer::read(BufferBox& b)
{
	b.set(buf, read_pos, 1.0f);
	moveReadPos(b.length);
}

void RingBuffer::write(BufferBox& b)
{
	int size_a = min(b.length, buf.length - write_pos);
	buf.set(b, write_pos, 1.0f);

	int size_b = b.length - size_a;
	if (size_b > 0){
		buf.set(b, -size_a, 1.0f);
	}

	moveWritePos(b.length);
}

void RingBuffer::peekRef(BufferBox &b, int size)
{
	if (size >= 0){
		size = min(size, available());
		size = min(size, buf.length - read_pos);
		b.set_as_ref(buf, read_pos, size);
	}else{
		size = min(-size, write_pos);
		b.set_as_ref(buf, write_pos - size, size);
	}
}

void RingBuffer::readRef(BufferBox &b, int size)
{
	peekRef(b, size);
	moveReadPos(b.length);
}

void RingBuffer::writeRef(BufferBox &b, int size)
{
	size = min(size, buf.length - write_pos);
	b.set_as_ref(buf, write_pos, size);
	moveWritePos(b.length);
}
