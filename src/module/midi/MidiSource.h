/*
 * MidiSource.h
 *
 *  Created on: 02.04.2018
 *      Author: michi
 */

#ifndef SRC_MODULE_MIDI_MIDISOURCE_H_
#define SRC_MODULE_MIDI_MIDISOURCE_H_

#include "../Module.h"
#include "../port/Port.h"

namespace tsunami {

class Song;
class MidiProduceData;

class MidiSource : public Module {
public:
	MidiSource();

	void _cdecl __init__();
	void _cdecl __delete__() override;

	MidiOutPort out{this};

	int read_midi(int port, MidiEventBuffer &midi) override;

	virtual int _cdecl read(MidiEventBuffer &midi);

	int produce_pos;
	int read_pos;
	virtual bool _cdecl on_produce(MidiProduceData &data) { return false; }


	void note(float pitch, float volume, int beats);
	void note_x(float pitch, float volume, int beats, int sub_beats, int beat_partition);
	void skip(int beats);
	void skip_x(int beats, int sub_beats, int beat_partition);
	MidiEventBuffer *bh_midi;
};

MidiSource *_cdecl CreateMidiSource(Session *session, const string &name);

}

#endif /* SRC_MODULE_MIDI_MIDISOURCE_H_ */
