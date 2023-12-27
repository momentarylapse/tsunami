/*
 * MidiAccumulator.h
 *
 *  Created on: 07.03.2019
 *      Author: michi
 */

#ifndef SRC_MODULE_MIDI_MIDIACCUMULATOR_H_
#define SRC_MODULE_MIDI_MIDIACCUMULATOR_H_


#include "../../data/midi/MidiData.h"
#include "../Module.h"
#include "../port/Port.h"
#include <mutex>

class MidiAccumulator : public Module {
public:
	MidiAccumulator();

	class Output : public Port {
	public:
		Output(MidiAccumulator *j);
		int read_midi(MidiEventBuffer &buf) override;
		MidiAccumulator *acc;
	};

	void _accumulate(bool enable);

	base::optional<int64> command(ModuleCommand cmd, int64 param) override;

	Port *source;
	MidiEventBuffer buffer;
	bool accumulating;
	std::mutex mtx_buf;
};

#endif /* SRC_MODULE_MIDI_MIDIACCUMULATOR_H_ */
