/*
 * BufferStreamer.cpp
 *
 *  Created on: 25.11.2015
 *      Author: michi
 */

#include "../Source/BufferStreamer.h"

BufferStreamer::BufferStreamer(BufferBox *b)
{
	buf = b;
	offset = 0;
}

void BufferStreamer::__init__(BufferBox* buf)
{
	new(this) BufferStreamer(buf);
}

void BufferStreamer::__delete__()
{
}

int BufferStreamer::read(BufferBox& _buf)
{
	int n = min(_buf.length, buf->length - offset);
	_buf.set(*buf, -offset, 1);
	offset += n;
	return n;
}

void BufferStreamer::reset()
{
	offset = 0;
}

Range BufferStreamer::range()
{
	return Range(0, buf->length);
}

void BufferStreamer::seek(int pos)
{
	offset = pos;
}

int BufferStreamer::getNumSamples()
{
	return buf->length;
}
