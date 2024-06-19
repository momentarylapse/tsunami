/*
 * MidiEffect.h
 *
 *  Created on: 10.09.2014
 *      Author: michi
 */

#ifndef SRC_MODULE_MIDI_MIDIEFFECT_H_
#define SRC_MODULE_MIDI_MIDIEFFECT_H_


#include "../../lib/base/base.h"
#include "../../data/Range.h"
#include "../Module.h"
#include "../port/Port.h"

namespace tsunami {

class Plugin;
class Track;
class TrackLayer;
class AudioBuffer;
class MidiEventBuffer;
class SongSelection;

class MidiEffect : public Module {
public:
	MidiEffect();

	void _cdecl __init__();
	void _cdecl __delete__() override;

	MidiOutPort out{this};
	MidiInPort in{this};

	virtual void _cdecl process(MidiEventBuffer &midi) {};

	void process_layer(TrackLayer *l, const SongSelection &sel);

	int read_midi(int port, MidiEventBuffer &midi) override;
};

MidiEffect *_cdecl CreateMidiEffect(Session *session, const string &name);

}

#endif /* SRC_MODULE_MIDI_MIDIEFFECT_H_ */
