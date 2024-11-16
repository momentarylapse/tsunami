//
// Created by Michael Ankele on 2024-11-16.
//

#ifndef MIDIEVENTRINGBUFFER_H
#define MIDIEVENTRINGBUFFER_H

#include "MidiEvent.h"
#include <atomic>

namespace tsunami {

class MidiEventRingBuffer {
public:
	explicit MidiEventRingBuffer(int size);

	void write(const Array<MidiEvent>& events);
	Array<MidiEvent> read();
	void clear();
	int available() const;

private:
	void _move_read_pos(int delta);
	void _move_write_pos(int delta);

	Array<MidiEvent> buffer;
	std::atomic<int> read_pos;
	std::atomic<int> write_pos;
	std::atomic<int> available_read;
	std::atomic<int> available_write;
};

} // tsunami

#endif //MIDIEVENTRINGBUFFER_H
