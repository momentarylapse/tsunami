/*
 * MidiEffect.h
 *
 *  Created on: 10.09.2014
 *      Author: michi
 */

#ifndef SRC_MODULE_MIDI_MIDIEFFECT_H_
#define SRC_MODULE_MIDI_MIDIEFFECT_H_


#include "../../lib/base/base.h"
#include "../../Data/Range.h"
#include "../Module.h"
#include "../Port/Port.h"

class Plugin;
class Track;
class TrackLayer;
class AudioBuffer;
class MidiNoteBuffer;
class SongSelection;

class MidiEffect : public Module {
public:
	MidiEffect();

	void _cdecl __init__();
	void _cdecl __delete__() override;

	Port *source;

	virtual void _cdecl process(MidiNoteBuffer *midi) {};

	void process_layer(TrackLayer *l, SongSelection &sel);

	class Output : public Port {
	public:
		Output(MidiEffect *fx);
		int read_midi(MidiEventBuffer &midi) override;

		MidiEffect *fx;
	};
};

MidiEffect *_cdecl CreateMidiEffect(Session *session, const string &name);

#endif /* SRC_MODULE_MIDI_MIDIEFFECT_H_ */
