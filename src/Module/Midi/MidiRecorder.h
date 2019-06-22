/*
 * MidiRecorder.h
 *
 *  Created on: 07.03.2019
 *      Author: michi
 */

#ifndef SRC_MODULE_MIDI_MIDIRECORDER_H_
#define SRC_MODULE_MIDI_MIDIRECORDER_H_


#include "../../Data/Midi/MidiData.h"
#include "../Module.h"
#include "../Port/Port.h"

class MidiRecorder : public Module {
public:
	MidiRecorder();

	class Output : public Port {
	public:
		Output(MidiRecorder *j);
		int read_midi(MidiEventBuffer &buf) override;
		MidiRecorder *rec;
	};

	void _accumulate(bool enable);

	int command(ModuleCommand cmd, int param) override;

	Port *source;
	MidiEventBuffer buffer;
	bool accumulating;
};

#endif /* SRC_MODULE_MIDI_MIDIRECORDER_H_ */
