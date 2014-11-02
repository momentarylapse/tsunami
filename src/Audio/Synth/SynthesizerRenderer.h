/*
 * SynthesizerRenderer.h
 *
 *  Created on: 02.11.2014
 *      Author: michi
 */

#ifndef SRC_AUDIO_SYNTH_SYNTHESIZERRENDERER_H_
#define SRC_AUDIO_SYNTH_SYNTHESIZERRENDERER_H_


#include "../AudioRenderer.h"

class Synthesizer;
class MidiNote;
class MidiSource;

class SynthesizerRenderer : public AudioRendererInterface
{
public:
	SynthesizerRenderer(Synthesizer *s);
	virtual ~SynthesizerRenderer();

	void __init__();
	virtual void __delete__();

	void setSynthesizer(Synthesizer *s);

	virtual int read(BufferBox &buf);
	void iterate(int samples);

	void set(float pitch, float volume, int offset);
	void reset();

	Synthesizer *s;
	Array<MidiNote> notes;

	MidiSource *source;
};

#endif /* SRC_AUDIO_SYNTH_SYNTHESIZERRENDERER_H_ */
