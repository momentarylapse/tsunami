/*
 * MidiJoiner.cpp
 *
 *  Created on: Jun 13, 2019
 *      Author: michi
 */

#include "MidiJoiner.h"
#include "../../data/base.h"
#include "../../data/midi/MidiData.h"

namespace tsunami {

MidiJoiner::MidiJoiner() : Module(ModuleCategory::Plumbing, "MidiJoiner") {
}

// if A or B is end-of-stream... just ignore and suck the other!
int MidiJoiner::read_midi(int port, MidiEventBuffer& buf) {
	bool first = true;
	int result = Return::NoSource;
	for (auto p: port_in) {
		if (!p->source)
			continue;
		if (first) {
			result = p->source->read_midi(buf);
			if (result == Return::NotEnoughData) {
				// better to wait...
				return result;
			}
			first = false;
		} else {
			// hmmm needs buffering if A has data, but B has none yet (e.g. input stream)
			MidiEventBuffer buf_b;
			buf_b.samples = buf.samples;
			int r = p->source->read_midi(buf_b);
			for (auto &n: buf_b)
				buf.add(n);
			result = max(result, r);
		}
	}
	return result;
}

}
