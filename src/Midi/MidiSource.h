/*
 * MidiSource.h
 *
 *  Created on: 29.04.2016
 *      Author: michi
 */

#ifndef SRC_MIDI_MIDISOURCE_H_
#define SRC_MIDI_MIDISOURCE_H_

#include "MidiData.h"

class MidiSource
{
public:
	MidiSource(){}
	virtual ~MidiSource(){}

	virtual int read(MidiRawData &midi) = 0;
	virtual void reset() = 0;
	virtual Range range() = 0;
	virtual int getPos() = 0;
	virtual void seek(int pos) = 0;
};

class MidiDataSource : public MidiSource
{
public:
	MidiDataSource(const MidiData &midi);
	virtual ~MidiDataSource();

	virtual int read(MidiRawData &midi);
	virtual void reset();
	virtual Range range();
	virtual int getPos();
	virtual void seek(int pos);

	MidiRawData midi;
	int offset;
};

#endif /* SRC_MIDI_MIDISOURCE_H_ */
