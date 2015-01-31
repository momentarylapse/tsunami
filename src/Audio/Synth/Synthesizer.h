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

class Range;
class BufferBox;
class MidiSource;
class MidiEvent;
class MidiNote;

class Synthesizer : public Configurable
{
public:
	Synthesizer();
	virtual ~Synthesizer();
	void __init__();
	virtual void __delete__();

	virtual void renderNote(BufferBox &buf, const Range &range, float pitch, float volume){}
	void renderMetronomeClick(BufferBox &buf, int pos, int level, float volume);

	virtual void render(BufferBox &buf);

	virtual void reset();

	int sample_rate;

	int keep_notes;

	virtual int read(BufferBox &buf);

	void add(const MidiEvent &e);
	void stopAll();
	void resetMidiData();

	bool auto_stop;

protected:
	void createNotes();
	void iterate(int samples);

	MidiSource *source;

	Array<MidiEvent> events[128];

	// accumulated...
	Array<MidiNote> cur_notes;
};

Synthesizer *CreateSynthesizer(const string &name);
Array<string> FindSynthesizers();

#endif /* SYNTHESIZER_H_ */
