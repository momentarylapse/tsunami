/*
 * MidiSucker.h
 *
 *  Created on: 09.03.2019
 *      Author: michi
 */

#ifndef SRC_MODULE_MIDI_MIDISUCKER_H_
#define SRC_MODULE_MIDI_MIDISUCKER_H_

#include "../Module.h"
#include "../port/Port.h"

namespace tsunami {

class MidiSucker : public Module {
public:
	MidiSucker();

	int update(int buffer_size);

	MidiInPort in{this, "in"};

	base::optional<int64> command(ModuleCommand cmd, int64 param) override;
};

}

#endif /* SRC_MODULE_MIDI_MIDISUCKER_H_ */
