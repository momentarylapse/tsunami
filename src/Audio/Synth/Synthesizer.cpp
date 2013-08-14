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

float pitch_to_freq(float pitch)
{
	return 440.0f * pow(2, (pitch - 69.0f) / 12.0f);
}

Synthesizer::Synthesizer()
{
	sample_rate = DEFAULT_SAMPLE_RATE;
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

/*void Synthesizer::AddTones(BufferBox& buf, Array<MidiNote>& notes)
{
}*/

void Synthesizer::reset()
{
}

void Synthesizer::set(float pitch, float volume, int offset)
{
}

void Synthesizer::iterate(int samples)
{
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
