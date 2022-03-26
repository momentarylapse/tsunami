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

	class Output : public Port {
	public:
		Output(MidiJoiner *j);
		int read_midi(MidiEventBuffer &buf) override;
		MidiJoiner *joiner;
	};

	Port *a, *b;
};

#endif /* SRC_MODULE_MIDI_MIDIJOINER_H_ */
