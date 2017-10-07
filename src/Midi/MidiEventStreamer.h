/*
 * MidiEventStreamer.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_MIDI_MIDIEVENTSTREAMER_H_
#define SRC_MIDI_MIDIEVENTSTREAMER_H_

#include "MidiSource.h"


class MidiEventStreamer : public MidiSource
{
public:
	MidiEventStreamer(const MidiEventBuffer &midi);
	virtual ~MidiEventStreamer();

	virtual int _cdecl read(MidiEventBuffer &midi);

	void _cdecl setData(const MidiEventBuffer &midi);

	void _cdecl seek(int pos);

	MidiEventBuffer midi;
	int offset;
	bool ignore_end;
};


#endif /* SRC_MIDI_MIDIEVENTSTREAMER_H_ */
