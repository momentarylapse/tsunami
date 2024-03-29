/*
 * MidiSucker.h
 *
 *  Created on: 09.03.2019
 *      Author: michi
 */

#ifndef SRC_MODULE_MIDI_MIDISUCKER_H_
#define SRC_MODULE_MIDI_MIDISUCKER_H_

#include "../Module.h"

class Port;

class MidiSucker : public Module {
public:
	MidiSucker();

	int update(int buffer_size);

	Port *source;

	base::optional<int64> command(ModuleCommand cmd, int64 param) override;
};

#endif /* SRC_MODULE_MIDI_MIDISUCKER_H_ */
