/*
 * Synthesizer.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SYNTHESIZER_H_
#define SYNTHESIZER_H_

#include "../../lib/base/base.h"

class Range;
class BufferBox;
class MidiNote;

class SynthesizerState
{
	Array<MidiNote> notes;
};

class Synthesizer
{
public:
	Synthesizer();
	virtual ~Synthesizer();
	void __init__();
	virtual void __delete__();

	virtual void AddTone(BufferBox &buf, const Range &range, float pitch, float volume){}
	void AddMetronomeClick(BufferBox &buf, int pos, int level, float volume);

	void reset();
	void set(float pitch, float volume, int offset);
	virtual void read(BufferBox &buf){}
	void iterate(int samples);

	virtual void configure(){}

	string name;
	int sample_rate;

	SynthesizerState state;
};

float pitch_to_freq(float pitch);

Synthesizer *CreateSynthesizer(const string &name);

#endif /* SYNTHESIZER_H_ */
