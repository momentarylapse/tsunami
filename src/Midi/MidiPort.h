/*
 * MidiPort.h
 *
 *  Created on: 29.04.2016
 *      Author: michi
 */

#ifndef SRC_MIDI_MIDIPORT_H_
#define SRC_MIDI_MIDIPORT_H_

#include "MidiData.h"

class BeatSource;
class MidiEventBuffer;

class MidiPort : public VirtualBase
{
public:
	MidiPort();
	virtual ~MidiPort(){}

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	virtual int _cdecl read(MidiEventBuffer &midi){ return 0; };
	virtual void _cdecl reset(){}

	static const int END_OF_STREAM;
	static const int NOT_ENOUGH_DATA;
};

#endif /* SRC_MIDI_MIDIPORT_H_ */
