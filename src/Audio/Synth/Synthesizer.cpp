/*
 * Synthesizer.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "Synthesizer.h"
#include "../../Data/AudioFile.h"
#include <math.h>

float pitch_to_freq(int pitch)
{
	return 440.0f * pow(2, (pitch - 69.0f) / 12.0f);
}

Synthesizer::Synthesizer()
{
	sample_rate = DEFAULT_SAMPLE_RATE;
	ref_count = 0;
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

void Synthesizer::ref()
{
	ref_count ++;
}

void Synthesizer::unref()
{
	ref_count --;
}

void Synthesizer::AddTone(BufferBox& buf, const Range& range, int pitch, float volume)
{
	AddToneFreq(buf, range, pitch_to_freq(pitch), volume);
}

void Synthesizer::AddClick(BufferBox &buf, int pos, int pitch, float volume)
{
	AddTone(buf, Range(pos, pos + sample_rate / 50), pitch, volume);
}

/*void Synthesizer::AddTones(BufferBox& buf, Array<MidiNote>& notes)
{
}*/


void Synthesizer::AddMetronomeClick(BufferBox &buf, int pos, int level, float volume)
{
	if (level == 0)
		AddClick(buf, pos, 81, volume);
	else
		AddClick(buf, pos, 74, volume * 0.5f);
}
