/*
 * MidiJoiner.h
 *
 *  Created on: Jun 13, 2019
 *      Author: michi
 */

#ifndef SRC_MODULE_MIDI_MIDIJOINER_H_
#define SRC_MODULE_MIDI_MIDIJOINER_H_


#include "../port/Port.h"
#include "../Module.h"

class MidiJoiner : public Module {
public:
	MidiJoiner();

	MidiOutPort out{this};
	MidiInPort in_a{this, "a"};
	MidiInPort in_b{this, "a"};

	int read_midi(int port, MidiEventBuffer &buf) override;
};

#endif /* SRC_MODULE_MIDI_MIDIJOINER_H_ */
