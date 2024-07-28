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

namespace tsunami {

MidiSucker::MidiSucker() :
	Module(ModuleCategory::Plumbing, "MidiSucker")
{
}

base::optional<int64> MidiSucker::command(ModuleCommand cmd, int64 param) {
	if (cmd == ModuleCommand::Suck) {
		return (int64)update((int)param);
	}
	return base::None;
}

int MidiSucker::update(int buffer_size) {
	if (!in.source)
		return Return::NoSource;
	MidiEventBuffer buf;
	buf.samples = buffer_size;
	return in.source->read_midi(buf);
}

}

