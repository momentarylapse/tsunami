/*
 * SynthesizerRenderer.cpp
 *
 *  Created on: 02.11.2014
 *      Author: michi
 */

#include "SynthesizerRenderer.h"
#include "../Synth/Synthesizer.h"



SynthesizerRenderer::SynthesizerRenderer(Synthesizer *_s)
{
	setSynthesizer(_s);
	setAutoStop(true);
}

SynthesizerRenderer::~SynthesizerRenderer()
{
}

void SynthesizerRenderer::__init__(Synthesizer *s)
{
	new(this) SynthesizerRenderer(s);
}

void SynthesizerRenderer::__delete__()
{
}

void SynthesizerRenderer::setSynthesizer(Synthesizer *_s)
{
	s = _s;
}

int SynthesizerRenderer::getSampleRate()
{
	if (s)
		return s->sample_rate;
	return DEFAULT_SAMPLE_RATE;
}

void SynthesizerRenderer::feed(const MidiRawData &data)
{
	if (s)
		s->feed(data);
}

void SynthesizerRenderer::endAllNotes()
{
	if (s)
		s->endAllNotes();
}

int SynthesizerRenderer::read(BufferBox &buf)
{
	if (!s)
		return 0;
	return s->read(buf);
}

void SynthesizerRenderer::reset()
{
	if (s)
		s->reset();
}

void SynthesizerRenderer::resetMidiData()
{
	if (s)
		s->resetMidiData();
}

void SynthesizerRenderer::setAutoStop(bool auto_stop)
{
	if (s)
		s->auto_stop = auto_stop;
}

