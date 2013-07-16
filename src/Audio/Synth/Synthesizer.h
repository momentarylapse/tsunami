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

class Synthesizer
{
public:
	Synthesizer();
	virtual ~Synthesizer();
	void __init__();
	virtual void __delete__();

	virtual void AddTone(BufferBox &buf, const Range &range, int pitch, float volume);
	virtual void AddToneFreq(BufferBox &buf, const Range &range, float freq, float volume){}
	virtual void AddClick(BufferBox &buf, int pos, int pitch, float volume);
	//void AddTones(BufferBox &buf, Array<MidiNote> &notes);
	void AddMetronomeClick(BufferBox &buf, int pos, int level, float volume);

	string name;
	int sample_rate;
};

float pitch_to_freq(int pitch);

Synthesizer *CreateSynthesizer(const string &name);

#endif /* SYNTHESIZER_H_ */
