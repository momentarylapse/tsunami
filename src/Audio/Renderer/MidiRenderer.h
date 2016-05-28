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

	void _cdecl __init__(Synthesizer *s, MidiSource *source);
	virtual void _cdecl __delete__();

	Synthesizer *_cdecl getSynthesizer(){ return s; }
	void _cdecl setSynthesizer(Synthesizer *s);

	virtual int _cdecl read(BufferBox &buf);
	virtual void _cdecl reset();

	virtual int _cdecl getSampleRate();

private:
	Synthesizer *s;
	MidiSource *source;
	bool samples_remaining;
};

#endif /* SRC_AUDIO_RENDERER_MIDIRENDERER_H_ */
