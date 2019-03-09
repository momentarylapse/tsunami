/*
 * MidiRecorder.cpp
 *
 *  Created on: 07.03.2019
 *      Author: michi
 */

#include "MidiRecorder.h"

#include "../Port/Port.h"
#include "../../Data/base.h"


int MidiRecorder::Output::read_midi(MidiEventBuffer& buf)
{
	if (!rec->source)
		return buf.samples;

	int r = rec->source->read_midi(buf);

	if (rec->accumulating and (r > 0)){
		rec->buffer.append(buf);
	}

	return r;
}

MidiRecorder::Output::Output(MidiRecorder *r) : Port(SignalType::MIDI, "out")
{
	rec = r;
}

MidiRecorder::MidiRecorder() :
	Module(ModuleType::PLUMBING, "MidiRecorder")
{
	port_out.add(new Output(this));
	port_in.add(InPortDescription(SignalType::MIDI, &source, "in"));
	source = nullptr;
	accumulating = false;
}

void MidiRecorder::accumulate(bool enable)
{
	accumulating = enable;
}

void MidiRecorder::reset_state()
{
	buffer.clear();
}

int MidiRecorder::command(ModuleCommand cmd, int param)
{
	if (cmd == ModuleCommand::ACCUMULATION_START){
		accumulate(true);
		return 0;
	}else if (cmd == ModuleCommand::ACCUMULATION_STOP){
		accumulate(false);
		return 0;
	}
	return COMMAND_NOT_HANDLED;
}
