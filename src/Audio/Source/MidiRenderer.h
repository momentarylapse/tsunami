/*
 * MidiRenderer.h
 *
 *  Created on: 02.11.2014
 *      Author: michi
 */

#ifndef SRC_AUDIO_SOURCE_MIDIRENDERER_H_
#define SRC_AUDIO_SOURCE_MIDIRENDERER_H_


#include "../Source/AudioSource.h"

class Synthesizer;
class MidiEvent;
class MidiSource;

class MidiRenderer : public AudioSource
{
public:
	MidiRenderer(Synthesizer *s, MidiSource *source);
	virtual ~MidiRenderer();

	void _cdecl __init__(Synthesizer *s, MidiSource *source);
	virtual void _cdecl __delete__();

	Synthesizer *_cdecl getSynthesizer(){ return s; }
	void _cdecl setSynthesizer(Synthesizer *s);

	virtual int _cdecl read(AudioBuffer &buf);
	virtual void _cdecl reset();

	virtual int _cdecl getSampleRate();

private:
	Synthesizer *s;
	MidiSource *source;
};

#endif /* SRC_AUDIO_SOURCE_MIDIRENDERER_H_ */
