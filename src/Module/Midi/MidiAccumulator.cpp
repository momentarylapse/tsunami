/*
 * MidiAccumulator.cpp
 *
 *  Created on: 07.03.2019
 *      Author: michi
 */

#include "MidiAccumulator.h"

#include "../Port/Port.h"
#include "../../Data/base.h"


int MidiAccumulator::Output::read_midi(MidiEventBuffer& buf) {
	if (!acc->source)
		return NO_SOURCE;

	int r = acc->source->read_midi(buf);

	if (acc->accumulating and (r > 0)) {
		std::lock_guard<std::mutex> lock(acc->mtx_buf);
		acc->buffer.append(buf);
	}

	return r;
}

MidiAccumulator::Output::Output(MidiAccumulator *a) : Port(SignalType::MIDI, "out") {
	acc = a;
}

MidiAccumulator::MidiAccumulator() :
	Module(ModuleCategory::PLUMBING, "MidiAccumulator")
{
	port_out.add(new Output(this));
	port_in.add({SignalType::MIDI, &source, "in"});
	source = nullptr;
	accumulating = false;
}

void MidiAccumulator::_accumulate(bool enable) {
	accumulating = enable;
}

int MidiAccumulator::command(ModuleCommand cmd, int param) {
	if (cmd == ModuleCommand::ACCUMULATION_START) {
		_accumulate(true);
		return 0;
	} else if (cmd == ModuleCommand::ACCUMULATION_STOP) {
		_accumulate(false);
		return 0;
	} else if (cmd == ModuleCommand::ACCUMULATION_CLEAR) {
		buffer.clear();
		return 0;
	} else if (cmd == ModuleCommand::ACCUMULATION_GET_SIZE) {
		return buffer.samples;
	}
	return COMMAND_NOT_HANDLED;
}
