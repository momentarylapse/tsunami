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

class MidiSplitter : public Module {
public:
	MidiSplitter();

	MidiOutPort out_a{this, "a", 0};
	MidiOutPort out_b{this, "b", 1};
	MidiInPort in{this};

	int read_midi(int port, MidiEventBuffer &buf) override;

	MidiEventBuffer buffer;
	int result = 0;
};

#endif /* SRC_MODULE_MIDI_MIDISPLITTER_H_ */
