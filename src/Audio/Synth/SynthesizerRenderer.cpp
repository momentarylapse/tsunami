/*
 * SynthesizerRenderer.cpp
 *
 *  Created on: 02.11.2014
 *      Author: michi
 */

#include "SynthesizerRenderer.h"
#include "Synthesizer.h"



const int MANY_SAMPLES = 0x7fffffff;

SynthesizerRenderer::SynthesizerRenderer(Synthesizer *_s)
{
	s = _s;
	source = NULL;
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
	this->~SynthesizerRenderer();
}

void SynthesizerRenderer::setSynthesizer(Synthesizer *_s)
{
	s = _s;
}


void SynthesizerRenderer::set(float pitch, float volume, int offset)
{
	// end active notes
	foreach(MidiNote &n, notes)
		if (n.pitch == pitch)
			if (n.range.is_inside(offset))
				n.range.num = offset - n.range.offset;

	// start a new note
	if (volume > 0){
		MidiNote n = MidiNote(Range(offset, MANY_SAMPLES), pitch, volume);
		notes.add(n);
	}
}

void SynthesizerRenderer::iterate(int samples)
{
	int keep_notes = 0;
	if (s)
		keep_notes = s->keep_notes;

	for (int i=0;i<notes.num;i++){
		notes[i].range.offset -= samples;
		if (notes[i].range.end() + keep_notes < 0){
			notes.erase(i);
			i --;
		}
	}
}

int SynthesizerRenderer::read(BufferBox &buf)
{
	if (!s)
		return 0;
	// get from source...
	buf.scale(0);

	foreach(MidiNote &n, notes)
		s->renderNote(buf, n.range, n.pitch, n.volume);

	iterate(buf.num);
	return buf.num;
}

void SynthesizerRenderer::reset()
{
	notes.clear();
}

