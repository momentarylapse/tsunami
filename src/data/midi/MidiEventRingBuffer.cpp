//
// Created by Michael Ankele on 2024-11-16.
//

#include "MidiEventRingBuffer.h"

namespace tsunami {

MidiEventRingBuffer::MidiEventRingBuffer(int size) {
	buffer.resize(size);
	clear();
}

void MidiEventRingBuffer::clear() {
	read_pos = 0;
	write_pos = 0;
	available_read = 0;
	available_write = buffer.num - 1;
}


Array<MidiEvent> MidiEventRingBuffer::read() {
	Array<MidiEvent> r;
	r.resize(available());

	int i0 = read_pos;
	for (int i=0; i<r.num; i++)
		r[i] = buffer[i0 + i];

	_move_read_pos(r.num);
	return r;
}

void MidiEventRingBuffer::_move_read_pos(int delta) {
	read_pos += delta;
	if (read_pos >= buffer.num)
		read_pos -= buffer.num;
	available_read -= delta;
	available_write += delta;
}

// use with care!
void MidiEventRingBuffer::_move_write_pos(int delta) {
	write_pos += delta;
	if (write_pos >= buffer.num)
		write_pos -= buffer.num;
	available_read += delta;
	available_write -= delta;
}

void MidiEventRingBuffer::write(const Array<MidiEvent>& events) {
	int n = min(available_write.load(), events.num);
	int i0 = write_pos;
	for (int i=0; i<n; i++)
		buffer[i0 + i] = events[i];
	_move_write_pos(n);
}

int MidiEventRingBuffer::available() const {
	return available_read;
}




} // tsunami