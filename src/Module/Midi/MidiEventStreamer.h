/*
 * MidiEventStreamer.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_MODULE_MIDI_MIDIEVENTSTREAMER_H_
#define SRC_MODULE_MIDI_MIDIEVENTSTREAMER_H_

#include "MidiSource.h"
#include "../../Data/Midi/MidiData.h"


class MidiEventStreamer : public MidiSource {
public:
	MidiEventStreamer(const MidiEventBuffer &midi);

	int read(MidiEventBuffer &midi) override;
	void reset() override;

	void _cdecl set_data(const MidiEventBuffer &midi);

	void set_pos(int pos);
	int get_pos();

	MidiEventBuffer midi;
	int offset;
	bool ignore_end;
};


#endif /* SRC_MODULE_MIDI_MIDIEVENTSTREAMER_H_ */
