/*
 * BufferStreamer.cpp
 *
 *  Created on: 25.11.2015
 *      Author: michi
 */

#include "../audio/BufferStreamer.h"

#include "../../data/audio/AudioBuffer.h"

namespace tsunami {

static bool CHEAT = true;

BufferStreamer::BufferStreamer(AudioBuffer *b) {
	buf = b;
	offset = 0;
}

void BufferStreamer::__init__(AudioBuffer* buf) {
	new(this) BufferStreamer(buf);
}

void BufferStreamer::__delete__() {
	this->BufferStreamer::~BufferStreamer();
}

int BufferStreamer::read(AudioBuffer& _buf) {
	int available = buf->length - offset;
	int n = min(_buf.length, available);
	if (n <= 0)
		return Return::EndOfStream;
	if (!CHEAT) {
		if (n < _buf.length)
			return Return::NotEnoughData;
	}
	_buf.set(*buf, -offset, 1);
	offset += n;
	return _buf.length;
}

void BufferStreamer::reset_state() {
	offset = 0;
}

/*void BufferStreamer::seek(int pos) {
	offset = pos;
}*/

}
