/*
 * BufferStreamer.cpp
 *
 *  Created on: 25.11.2015
 *      Author: michi
 */

#include "../Source/BufferStreamer.h"

static bool CHEAT = true;

BufferStreamer::BufferStreamer(AudioBuffer *b)
{
	buf = b;
	offset = 0;
}

void BufferStreamer::__init__(AudioBuffer* buf)
{
	new(this) BufferStreamer(buf);
}

void BufferStreamer::__delete__()
{
	this->~BufferStreamer();
}

int BufferStreamer::read(AudioBuffer& _buf)
{
	int available = buf->length - offset;
	int n = min(_buf.length, available);
	if (n <= 0)
		return END_OF_STREAM;
	if (!CHEAT){
		if (n < _buf.length)
			return NOT_ENOUGH_DATA;
	}
	_buf.set(*buf, -offset, 1);
	offset += n;
	return _buf.length;
}

void BufferStreamer::reset()
{
	offset = 0;
}

void BufferStreamer::seek(int pos)
{
	offset = pos;
}
