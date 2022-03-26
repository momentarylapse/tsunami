/*
 * MidiEventStreamer.cpp
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#include "MidiEventStreamer.h"

#include "../beats/BeatSource.h"


MidiEventStreamer::MidiEventStreamer(const MidiEventBuffer& _midi) {
	module_class = "MidiEventStreamer";
	midi = _midi;
	offset = 0;
	ignore_end = false;
}

int MidiEventStreamer::read(MidiEventBuffer& _midi) {
	int n = min(midi.samples - offset, _midi.samples);
	if (ignore_end)
		n = _midi.samples;
	if ((n <= 0) and !ignore_end)
		return Port::END_OF_STREAM;

	Range r = Range(offset, n);
	//midi.read(_midi, r);
	for (MidiEvent &e : midi)
		if (r.is_inside(e.pos))
			_midi.add(MidiEvent(e.pos - offset, e.pitch, e.volume));
	offset += n;
	return n;
}

void MidiEventStreamer::reset() {
	offset = 0;
}

void MidiEventStreamer::set_data(const MidiEventBuffer &_midi) {
	midi = _midi;
}

void MidiEventStreamer::set_pos(int pos) {
	offset = pos;
}

int MidiEventStreamer::get_pos() {
	return offset;
}




