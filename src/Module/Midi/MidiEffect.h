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
#include "../Port/MidiPort.h"

class Plugin;
class Track;
class TrackLayer;
class AudioBuffer;
class MidiNoteBuffer;
class SongSelection;
class Song;

class MidiEffect : public Module
{
public:
	MidiEffect();
	virtual ~MidiEffect();

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	bool only_on_selection;
	Range range;

	MidiPort *source;
	void set_source(MidiPort *s);

	virtual void _cdecl process(MidiNoteBuffer *midi){};

	void process_layer(TrackLayer *l, const SongSelection &sel);

	void prepare();
	void apply(MidiNoteBuffer &midi, Track *t, bool log_error);

	int bh_offset;
	void note(float pitch, float volume, int beats);
	void note_x(float pitch, float volume, int beats, int sub_beats, int beat_partition);
	void skip(int beats);
	void skip_x(int beats, int sub_beats, int beat_partition);
	Song *bh_song;
	MidiNoteBuffer *bh_midi;

	class Output : public MidiPort
	{
	public:
		Output(MidiEffect *fx);
		virtual ~Output(){}
		virtual int _cdecl read(MidiEventBuffer &midi);
		virtual void _cdecl reset();

		MidiEffect *fx;
	};
	Output *out;
};

MidiEffect *_cdecl CreateMidiEffect(Session *session, const string &name);

#endif /* SRC_MODULE_MIDI_MIDIEFFECT_H_ */
