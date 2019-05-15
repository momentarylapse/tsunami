/*
 * SampleSynthesizer.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "SampleSynthesizer.h"
#include "../../Data/SampleRef.h"

SampleSynthesizer::SampleSynthesizer()
{
	module_subtype = "Sample";
}

SampleSynthesizer::~SampleSynthesizer()
{
	for (SampleRef *s : samples)
		if (s)
			delete(s);
}

void SampleSynthesizer::__init__()
{
	new(this) SampleSynthesizer;
}

void SampleSynthesizer::__delete__()
{
	this->SampleSynthesizer::~SampleSynthesizer();
}

void SampleSynthesizer::renderNote(AudioBuffer& buf, const Range& range, float pitch, float volume)
{
	if ((pitch < 0) or (pitch >= samples.num))
		return;
	SampleRef *s = samples[pitch];
	if (!s)
		return;
	buf.add(*s->buf, range.start(), volume);
}
