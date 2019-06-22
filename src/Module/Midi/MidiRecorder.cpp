/*
 * MidiRecorder.cpp
 *
 *  Created on: 07.03.2019
 *      Author: michi
 */

#include "MidiRecorder.h"

#include "../Port/Port.h"
#include "../../Data/base.h"


int MidiRecorder::Output::read_midi(MidiEventBuffer& buf) {
	if (!rec->source)
		return END_OF_STREAM;

	int r = rec->source->read_midi(buf);

	if (rec->accumulating and (r > 0)) {
		rec->buffer.append(buf);
	}

	return r;
}

MidiRecorder::Output::Output(MidiRecorder *r) : Port(SignalType::MIDI, "out") {
	rec = r;
}

MidiRecorder::MidiRecorder() :
	Module(ModuleType::PLUMBING, "MidiRecorder")
{
	port_out.add(new Output(this));
	port_in.add({SignalType::MIDI, &source, "in"});
	source = nullptr;
	accumulating = false;
}

void MidiRecorder::_accumulate(bool enable) {
	accumulating = enable;
}

int MidiRecorder::command(ModuleCommand cmd, int param) {
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
