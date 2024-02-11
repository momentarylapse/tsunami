/*
 * MidiEventStreamer.h
 *
 *  Created on: 07.10.2017
 *      Author: michi
 */

#ifndef SRC_MODULE_MIDI_MIDIEVENTSTREAMER_H_
#define SRC_MODULE_MIDI_MIDIEVENTSTREAMER_H_

#include "MidiSource.h"
#include "../../data/midi/MidiData.h"


class MidiEventStreamer : public MidiSource {
public:
	MidiEventStreamer();

	int read(MidiEventBuffer &midi) override;
	void reset_state() override;

	void set_data(const MidiEventBuffer &midi);

	void set_pos(int pos);
	int get_pos() const;

	MidiEventBuffer midi;
	int offset;
	bool ignore_end;
	bool loop;
};


#endif /* SRC_MODULE_MIDI_MIDIEVENTSTREAMER_H_ */
