/*
 * MidiEffect.h
 *
 *  Created on: 10.09.2014
 *      Author: michi
 */

#ifndef MIDIEFFECT_H_
#define MIDIEFFECT_H_


#include "../lib/base/base.h"
#include "../Data/Range.h"
#include "Configurable.h"

class Plugin;
class Track;
class AudioBuffer;
class MidiNoteBuffer;
class SongSelection;

namespace Script{
class Script;
class Type;
};

class MidiEffect : public Configurable
{
public:
	MidiEffect();
	MidiEffect(Plugin *p);
	virtual ~MidiEffect();

	void _cdecl __init__();
	virtual void _cdecl __delete__();

	bool only_on_selection;
	Range range;
	Plugin *plugin;
	bool usable;
	bool enabled;

	virtual void _cdecl process(MidiNoteBuffer *midi){};

	void process_track(Track *t, const SongSelection &sel);

	void prepare();
	void apply(MidiNoteBuffer &midi, Track *t, bool log_error);

	string GetError();

	int bh_offset;
	void note(float pitch, float volume, int beats);
	void note_x(float pitch, float volume, int beats, int sub_beats, int beat_partition);
	void skip(int beats);
	void skip_x(int beats, int sub_beats, int beat_partition);
	Song *bh_song;
	MidiNoteBuffer *bh_midi;
};

MidiEffect *_cdecl CreateMidiEffect(const string &name, Song *song);

#endif /* MIDIEFFECT_H_ */
