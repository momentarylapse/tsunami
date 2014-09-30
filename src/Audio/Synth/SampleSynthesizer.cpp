/*
 * SampleSynthesizer.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "SampleSynthesizer.h"

SampleSynthesizer::SampleSynthesizer()
{
	name = "Sample";
}

SampleSynthesizer::~SampleSynthesizer()
{
	foreach(SampleRef *s, samples)
		if (s)
			delete(s);
}

void SampleSynthesizer::__init__()
{
	new(this) SampleSynthesizer;
}

void SampleSynthesizer::__delete__()
{
	foreach(SampleRef *s, samples)
		if (s)
			delete(s);
	samples.clear();
}

void SampleSynthesizer::RenderNote(BufferBox& buf, const Range& range, float pitch, float volume)
{
	if ((pitch < 0) || (pitch >= samples.num))
		return;
	SampleRef *s = samples[pitch];
	if (!s)
		return;
	buf.add(*s->buf, range.start(), volume, 0);
}
