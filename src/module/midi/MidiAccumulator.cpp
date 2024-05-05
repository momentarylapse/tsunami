/*
 * MidiAccumulator.cpp
 *
 *  Created on: 07.03.2019
 *      Author: michi
 */

#include "MidiAccumulator.h"

#include "../port/Port.h"
#include "../../data/base.h"


int MidiAccumulator::read_midi(int port, MidiEventBuffer& buf) {
	if (!in.source)
		return NO_SOURCE;

	int r = in.source->read_midi(buf);

	if (accumulating and (r > 0)) {
		std::lock_guard<std::mutex> lock(mtx_buf);
		buffer.append(buf);
	}

	return r;
}

MidiAccumulator::MidiAccumulator() :
	Module(ModuleCategory::PLUMBING, "MidiAccumulator")
{
	accumulating = false;
}

void MidiAccumulator::_accumulate(bool enable) {
	accumulating = enable;
}

base::optional<int64> MidiAccumulator::command(ModuleCommand cmd, int64 param) {
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
	return base::None;
}
