/*
 * Synthesizer.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SYNTHESIZER_H_
#define SYNTHESIZER_H_

#include "../../lib/base/base.h"
#include "../AudioRenderer.h"

class Range;
class BufferBox;
class MidiNote;
class MidiSource;

class Synthesizer : public AudioRendererInterface
{
public:
	Synthesizer();
	virtual ~Synthesizer();
	void __init__();
	virtual void __delete__();

	virtual void RenderNote(BufferBox &buf, const Range &range, float pitch, float volume){}
	void RenderMetronomeClick(BufferBox &buf, int pos, int level, float volume);

	void reset();
	void set(float pitch, float volume, int offset);
	virtual int read(BufferBox &buf);
	void iterate(int samples);

	virtual void OnConfigure(){}

	string name;
	int sample_rate;

	MidiSource *source;

	int keep_notes;
	Array<MidiNote> notes;
};

float pitch_to_freq(float pitch);

Synthesizer *CreateSynthesizer(const string &name);
Array<string> FindSynthesizers();

#endif /* SYNTHESIZER_H_ */
