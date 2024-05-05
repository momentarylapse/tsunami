/*
 * MidiSplitter.cpp
 *
 *  Created on: Jun 13, 2019
 *      Author: michi
 */

#include "MidiSplitter.h"
#include "../../lib/base/algo.h"
#include "../../data/base.h"
#include "../../data/midi/MidiData.h"

// TODO: replace by multi-connection ports/auto splitting

MidiSplitter::MidiSplitter() : Module(ModuleCategory::PLUMBING, "MidiSplitter") {
}

int MidiSplitter::read_midi(int port, MidiEventBuffer& buf) {
	if (!in.source)
		return NO_SOURCE;

	int first_connected_port = base::find_index_if(port_out, [] (auto p) {
		return p->_connection_count > 0;
	});

	if (port == first_connected_port) {
		buffer.clear();
		buffer.samples = buf.samples;
		result = in.source->read_midi(buffer);
	}

	if (buf.samples != buffer.samples)
		return NOT_ENOUGH_DATA;
	int samples = buf.samples;
	buf = buffer;
	buf.samples = samples;
	return result;
}
