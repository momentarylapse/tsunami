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
class BufferBox;
class MidiNoteData;

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

	virtual void _cdecl process(MidiNoteData *midi){};

	void DoProcessTrack(Track *t, const Range &r);

	void Prepare();
	void Apply(MidiNoteData &midi, Track *t, bool log_error);

	string GetError();
};

MidiEffect *_cdecl CreateMidiEffect(const string &name);

#endif /* MIDIEFFECT_H_ */
