/*
 * MidiSucker.cpp
 *
 *  Created on: 09.03.2019
 *      Author: michi
 */

#include "MidiSucker.h"
#include "../port/Port.h"
#include "../../data/base.h"
#include "../../data/midi/MidiData.h"


MidiSucker::MidiSucker() :
	Module(ModuleCategory::PLUMBING, "MidiSucker")
{
}

base::optional<int64> MidiSucker::command(ModuleCommand cmd, int64 param) {
	if (cmd == ModuleCommand::SUCK) {
		return (int64)update((int)param);
	}
	return base::None;
}

int MidiSucker::update(int buffer_size) {
	if (!in.source)
		return Port::NO_SOURCE;
	MidiEventBuffer buf;
	buf.samples = buffer_size;
	return in.source->read_midi(buf);
}

