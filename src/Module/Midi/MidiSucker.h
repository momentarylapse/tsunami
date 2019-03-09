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
class MidiSuckerThread;

class MidiSucker : public Module
{
	friend class MidiSuckerThread;
public:
	MidiSucker();
	~MidiSucker() override;

	void set_buffer_size(int size);

	void start();
	void stop();

	int update();

	static const int DEFAULT_BUFFER_SIZE;

	Port *source;
	bool running;
	int buffer_size;
	float no_data_wait;

	MidiSuckerThread *thread;
	void command(ModuleCommand cmd) override;
};

#endif /* SRC_MODULE_MIDI_MIDISUCKER_H_ */
