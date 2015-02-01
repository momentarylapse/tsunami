/*
 * SynthesizerRenderer.cpp
 *
 *  Created on: 02.11.2014
 *      Author: michi
 */

#include "SynthesizerRenderer.h"
#include "Synthesizer.h"



SynthesizerRenderer::SynthesizerRenderer(Synthesizer *_s)
{
	auto_stop = false;
	setSynthesizer(_s);
}

SynthesizerRenderer::~SynthesizerRenderer()
{
}

void SynthesizerRenderer::__init__()
{
	new(this) SynthesizerRenderer(NULL);
}

void SynthesizerRenderer::__delete__()
{
}

void SynthesizerRenderer::setSynthesizer(Synthesizer *_s)
{
	s = _s;
	if (s)
		sample_rate = s->sample_rate;
	else
		sample_rate = DEFAULT_SAMPLE_RATE;
}


void SynthesizerRenderer::add(const MidiEvent &e)
{
	if (s)
		s->add(e);
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

void SynthesizerRenderer::resetMidiData()
{
	if (s)
		s->resetMidiData();
}

