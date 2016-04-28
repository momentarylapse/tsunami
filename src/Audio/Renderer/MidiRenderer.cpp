/*
 * MidiRenderer.cpp
 *
 *  Created on: 02.11.2014
 *      Author: michi
 */

#include "../Synth/Synthesizer.h"
#include "MidiRenderer.h"



MidiRenderer::MidiRenderer(Synthesizer *_s)
{
	auto_stop = true;
	setSynthesizer(_s);
}

MidiRenderer::~MidiRenderer()
{
}

void MidiRenderer::__init__(Synthesizer *s)
{
	new(this) MidiRenderer(s);
}

void MidiRenderer::__delete__()
{
}

void MidiRenderer::setSynthesizer(Synthesizer *_s)
{
	s = _s;
	if (s)
		s->auto_stop = auto_stop;
}

int MidiRenderer::getSampleRate()
{
	if (s)
		return s->sample_rate;
	return DEFAULT_SAMPLE_RATE;
}

void MidiRenderer::feed(const MidiRawData &data)
{
	if (s)
		s->feed(data);
}

void MidiRenderer::endAllNotes()
{
	if (s)
		s->endAllNotes();
}

int MidiRenderer::read(BufferBox &buf)
{
	if (!s)
		return 0;
	return s->read(buf);
}

void MidiRenderer::reset()
{
	if (s)
		s->reset();
}

void MidiRenderer::resetMidiData()
{
	if (s)
		s->resetMidiData();
}

void MidiRenderer::setAutoStop(bool _auto_stop)
{
	auto_stop = _auto_stop;
	if (s)
		s->auto_stop = _auto_stop;
}

