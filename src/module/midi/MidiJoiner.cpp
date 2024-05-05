/*
 * MidiJoiner.cpp
 *
 *  Created on: Jun 13, 2019
 *      Author: michi
 */

#include "MidiJoiner.h"
#include "../../data/base.h"
#include "../../data/midi/MidiData.h"

MidiJoiner::MidiJoiner() : Module(ModuleCategory::PLUMBING, "MidiJoiner") {
}

// if A or B is end-of-stream... just ignore and suck the other!
int MidiJoiner::read_midi(int port, MidiEventBuffer& buf) {
	auto pa = in_a.source;
	auto pb = in_b.source;
	if (pa and pb) {
		int ra = pa->read_midi(buf);
		if (ra == NOT_ENOUGH_DATA) {
			// better to wait...
			return ra;
		}
		// hmmm needs buffering if A has data, but B has none yet (e.g. input stream)
		MidiEventBuffer buf_b;
		buf_b.samples = buf.samples;
		int rb = pb->read_midi(buf_b);
		for (auto &n: buf_b)
			buf.add(n);
		return max(ra, rb);
	} else if (pa) {
		return pa->read_midi(buf);
	} else if (pb) {
		return pb->read_midi(buf);
	}

	// no sources present
	return NO_SOURCE;
}
