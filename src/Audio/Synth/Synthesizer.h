/*
 * Synthesizer.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SYNTHESIZER_H_
#define SYNTHESIZER_H_

#include "../../lib/base/base.h"
#include "../../Plugins/Configurable.h"
#include "../../Data/MidiData.h"

class Range;
class BufferBox;
class MidiSource;
class PluginManager;

class Synthesizer : public Configurable
{
	friend class PluginManager;
public:
	Synthesizer();
	virtual ~Synthesizer();
	void __init__();
	virtual void __delete__();

	virtual void render(BufferBox &buf){}

	int sample_rate;
	void setSampleRate(int sample_rate);

	int keep_notes;

	virtual int read(BufferBox &buf);

	void feed(const MidiData &events);
	void addMetronomeClick(int pos, int level, float volume);
	void add(const MidiEvent &e);
	void endAllNotes();
	void resetMidiData();
	void reset();

	void iterateEvents(int samples);

	bool hasEnded();
	bool auto_stop;

protected:

	MidiSource *source;

	MidiData events;

	Set<int> active_pitch;
	Array<int> delete_me;
	void enablePitch(int pitch, bool enable);

	float delta_phi[128];
};

Synthesizer *CreateSynthesizer(const string &name);
Array<string> FindSynthesizers();

#endif /* SYNTHESIZER_H_ */
