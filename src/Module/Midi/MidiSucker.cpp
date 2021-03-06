/*
 * MidiSucker.cpp
 *
 *  Created on: 09.03.2019
 *      Author: michi
 */

#include "MidiSucker.h"
#include "../Port/Port.h"
#include "../../Data/base.h"
#include "../../Data/Midi/MidiData.h"


MidiSucker::MidiSucker() :
	Module(ModuleCategory::PLUMBING, "MidiSucker")
{
	port_in.add({SignalType::MIDI, &source, "in"});
	source = nullptr;
}

int MidiSucker::command(ModuleCommand cmd, int param) {
	if (cmd == ModuleCommand::SUCK) {
		return update(param);
	}
	return COMMAND_NOT_HANDLED;
}

int MidiSucker::update(int buffer_size) {
	if (!source)
		return Port::NO_SOURCE;
	MidiEventBuffer buf;
	buf.samples = buffer_size;
	return source->read_midi(buf);
}

