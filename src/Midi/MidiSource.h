/*
 * MidiSource.h
 *
 *  Created on: 29.04.2016
 *      Author: michi
 */

#ifndef SRC_MIDI_MIDISOURCE_H_
#define SRC_MIDI_MIDISOURCE_H_

#include "MidiData.h"

class BeatSource;

class MidiSource : public VirtualBase
{
public:
	MidiSource();
	virtual ~MidiSource(){}

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	virtual int _cdecl read(MidiEventBuffer &midi){ return 0; };

	BeatSource *beat_source;
	void setBeatSource(BeatSource *beat_source);

	static const int END_OF_STREAM;
	static const int NOT_ENOUGH_DATA;
};

class BeatMidifier : public MidiSource
{
public:
	virtual int _cdecl read(MidiEventBuffer &midi);
};

#endif /* SRC_MIDI_MIDISOURCE_H_ */
