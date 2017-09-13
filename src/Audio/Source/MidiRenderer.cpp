/*
 * MidiRenderer.cpp
 *
 *  Created on: 02.11.2014
 *      Author: michi
 */

#include "../Source/MidiRenderer.h"

#include "../Synth/Synthesizer.h"



MidiRenderer::MidiRenderer(Synthesizer *_s, MidiSource *_source)
{
	samples_remaining = -1;
	source = _source;
	setSynthesizer(_s);
}

MidiRenderer::~MidiRenderer()
{
}

void MidiRenderer::__init__(Synthesizer *s, MidiSource *source)
{
	new(this) MidiRenderer(s, source);
}

void MidiRenderer::__delete__()
{
	this->MidiRenderer::~MidiRenderer();
}

void MidiRenderer::setSynthesizer(Synthesizer *_s)
{
	s = _s;
}

int MidiRenderer::getSampleRate()
{
	if (s)
		return s->sample_rate;
	return DEFAULT_SAMPLE_RATE;
}

int MidiRenderer::read(AudioBuffer &buf)
{
	if (!s or !source)
		return 0;
	return s->read(buf, source);
}

void MidiRenderer::reset()
{
	if (s)
		s->reset();
}

