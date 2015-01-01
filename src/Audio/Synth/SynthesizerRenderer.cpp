/*
 * SynthesizerRenderer.cpp
 *
 *  Created on: 02.11.2014
 *      Author: michi
 */

#include "SynthesizerRenderer.h"
#include "Synthesizer.h"



const int MANY_SAMPLES = 0x60000000;

SynthesizerRenderer::SynthesizerRenderer(Synthesizer *_s)
{
	source = NULL;
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
	this->~SynthesizerRenderer();
}

void SynthesizerRenderer::setSynthesizer(Synthesizer *_s)
{
	s = _s;
	if (s)
		sample_rate = s->sample_rate;
	else
		sample_rate = DEFAULT_SAMPLE_RATE;
}


void SynthesizerRenderer::add(int offset, float pitch, float volume)
{
	MidiEvent ee = MidiEvent(offset, pitch, volume);

	for (int i=events.num-1; i>=0; i--)
		if (events[i].pos <= offset){
			events.insert(ee, i+1);
			return;
		}
	events.add(ee);
}

void SynthesizerRenderer::stopAll()
{
	foreach(MidiNote &n, cur_notes){
		if (n.range.end() > 0){
			add(0, n.pitch, 0);
		}
	}
}

void SynthesizerRenderer::iterate(int samples)
{
	int keep_notes = 0;
	if (s)
		keep_notes = s->keep_notes;

	for (int i=0;i<cur_notes.num;i++){
		cur_notes[i].range.offset -= samples;
		if (cur_notes[i].range.end() + keep_notes < 0){
			cur_notes.erase(i);
			i --;
		}
	}
}

void SynthesizerRenderer::createNotes()
{
	foreach(MidiEvent &e, events){
		if (e.volume > 0){
			// start new note
			MidiNote n = MidiNote(Range(e.pos, MANY_SAMPLES), e.pitch, e.volume);
			cur_notes.add(n);
		}else{
			// stop note
			foreach(MidiNote &n, cur_notes)
				if ((e.pitch == n.pitch) and (n.range.is_inside(e.pos)))
					n.range.set_end(e.pos);
		}
	}

	events.clear();
}

int SynthesizerRenderer::read(BufferBox &buf)
{
	if (!s)
		return 0;
	// get from source...
	buf.scale(0);

	createNotes();

	if ((auto_stop) and (cur_notes.num == 0))
		return 0;

	foreach(MidiNote &n, cur_notes)
		s->renderNote(buf, n.range, n.pitch, n.volume);

	iterate(buf.num);

	return buf.num;
}

void SynthesizerRenderer::reset()
{
	events.clear();
	cur_notes.clear();
}

