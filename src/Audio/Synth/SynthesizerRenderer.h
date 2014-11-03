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
class MidiEvent;
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

	void add(int offset, float pitch, float volume);
	void reset();

	bool auto_stop;

private:
	void createNotes();
	void iterate(int samples);

	Synthesizer *s;
	MidiSource *source;

	Array<MidiEvent> events;

	// accumulated...
	Array<MidiNote> cur_notes;
};

#endif /* SRC_AUDIO_SYNTH_SYNTHESIZERRENDERER_H_ */
