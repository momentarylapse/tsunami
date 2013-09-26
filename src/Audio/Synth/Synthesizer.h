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

	virtual void Configure(){}
	virtual void DataToDialog(){}
	virtual void DataFromDialog(){}
	virtual void Reset();
	virtual void ResetConfig(){}

	void options_to_string();
	void options_from_string();

	string name;
	string options;
	int sample_rate;

	MidiSource *source;

	int keep_notes;
	Array<MidiNote> notes;
};

float pitch_to_freq(float pitch);
int pitch_get_octave(int pitch);
int pitch_from_octave_and_rel(int rel, int octave);
int pitch_to_rel(int pitch);
string rel_pitch_name(int pitch_rel);
string pitch_name(int pitch);

Synthesizer *CreateSynthesizer(const string &name);
Array<string> FindSynthesizers();

#endif /* SYNTHESIZER_H_ */
