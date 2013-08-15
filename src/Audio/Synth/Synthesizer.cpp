/*
 * Synthesizer.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "Synthesizer.h"
#include "DummySynthesizer.h"
#include "SampleSynthesizer.h"
#include "../../Data/AudioFile.h"
#include "../../Tsunami.h"
#include "../../Stuff/Log.h"
#include <math.h>

const int MANY_SAMPLES = 0x7fffffff;

float pitch_to_freq(float pitch)
{
	return 440.0f * pow(2, (pitch - 69.0f) / 12.0f);
}

Synthesizer::Synthesizer()
{
	sample_rate = DEFAULT_SAMPLE_RATE;
	keep_notes = 0;
	reset();
}

Synthesizer::~Synthesizer()
{
}

void Synthesizer::__init__()
{
	new(this) Synthesizer;
}

void Synthesizer::__delete__()
{
}

void Synthesizer::reset()
{
	notes.clear();
	keep_notes = 0;
}

void Synthesizer::set(float pitch, float volume, int offset)
{
	// end active notes
	foreach(MidiNote &n, notes)
		if (n.pitch == pitch)
			if (n.range.is_inside(offset))
				n.range.num = offset - n.range.offset;

	// start a new note
	if (volume > 0){
		MidiNote n = MidiNote(Range(offset, MANY_SAMPLES), pitch, volume);
	}
}

void Synthesizer::iterate(int samples)
{
	for (int i=0;i<notes.num;i++){
		notes[i].range.offset -= samples;
		if (notes[i].range.end() + keep_notes < 0){
			notes.erase(i);
			i --;
		}
	}
}

void Synthesizer::read(BufferBox &buf)
{
	// get from source...

	foreach(MidiNote &n, notes)
		AddTone(buf, n.range, n.pitch, n.volume);

	iterate(buf.num);
}

void Synthesizer::AddMetronomeClick(BufferBox &buf, int pos, int level, float volume)
{
	if (level == 0)
		AddTone(buf, Range(pos, 0), 81, volume);
	else
		AddTone(buf, Range(pos, 0), 74, volume * 0.5f);
}


// factory
Synthesizer *CreateSynthesizer(const string &name)
{
	if ((name == "Dummy") || (name == ""))
		return new DummySynthesizer;
	if (name == "Sample")
		return new SampleSynthesizer;
	tsunami->log->Error(_("unbekannter Synthesizer: ") + name);
	return new DummySynthesizer;
}
