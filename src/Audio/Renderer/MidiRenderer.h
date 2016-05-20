/*
 * MidiRenderer.h
 *
 *  Created on: 02.11.2014
 *      Author: michi
 */

#ifndef SRC_AUDIO_RENDERER_MIDIRENDERER_H_
#define SRC_AUDIO_RENDERER_MIDIRENDERER_H_


#include "AudioRenderer.h"

class Synthesizer;
class MidiEvent;
class MidiSource;

class MidiRenderer : public AudioRenderer
{
public:
	MidiRenderer(Synthesizer *s, MidiSource *source);
	virtual ~MidiRenderer();

	void __init__(Synthesizer *s, MidiSource *source);
	virtual void __delete__();

	Synthesizer *getSynthesizer(){ return s; }
	void setSynthesizer(Synthesizer *s);

	virtual int read(BufferBox &buf);
	virtual void reset();

	virtual int getSampleRate();

private:
	Synthesizer *s;
	MidiSource *source;
	bool samples_remaining;
};

#endif /* SRC_AUDIO_RENDERER_MIDIRENDERER_H_ */
