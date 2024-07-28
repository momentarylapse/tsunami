/*
 * MidiAccumulator.cpp
 *
 *  Created on: 07.03.2019
 *      Author: michi
 */

#include "MidiAccumulator.h"

#include "../port/Port.h"
#include "../../data/base.h"

namespace tsunami {

int MidiAccumulator::read_midi(int port, MidiEventBuffer& buf) {
	if (!in.source)
		return Return::NoSource;

	int r = in.source->read_midi(buf);

	if (accumulating and (r > 0)) {
		std::lock_guard<std::mutex> lock(mtx_buf);
		buffer.append(buf);
	}

	return r;
}

MidiAccumulator::MidiAccumulator() :
	Module(ModuleCategory::Plumbing, "MidiAccumulator")
{
	accumulating = false;
}

void MidiAccumulator::_accumulate(bool enable) {
	accumulating = enable;
}

base::optional<int64> MidiAccumulator::command(ModuleCommand cmd, int64 param) {
	if (cmd == ModuleCommand::AccumulationStart) {
		_accumulate(true);
		return 0;
	} else if (cmd == ModuleCommand::AccumulationStop) {
		_accumulate(false);
		return 0;
	} else if (cmd == ModuleCommand::AccumulationClear) {
		buffer.clear();
		return 0;
	} else if (cmd == ModuleCommand::AccumulationGetSize) {
		return buffer.samples;
	}
	return base::None;
}

}
