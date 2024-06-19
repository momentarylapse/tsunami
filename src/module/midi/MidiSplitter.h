/*
 * MidiSplitter.h
 *
 *  Created on: May 05, 2024
 *      Author: michi
 */

#ifndef SRC_MODULE_MIDI_MIDISPLITTER_H_
#define SRC_MODULE_MIDI_MIDISPLITTER_H_


#include "../Module.h"
#include "../port/Port.h"
#include "../../data/midi/MidiData.h"

namespace tsunami {

class MidiSplitter : public Module {
public:
	MidiSplitter();

	MidiOutPort out_a{this, "a"};
	MidiOutPort out_b{this, "b"};
	MidiOutPort out_c{this, "c"};
	MidiOutPort out_d{this, "d"};
	MidiInPort in{this};

	int read_midi(int port, MidiEventBuffer &buf) override;

	MidiEventBuffer buffer;
	int result = 0;
};

}

#endif /* SRC_MODULE_MIDI_MIDISPLITTER_H_ */
