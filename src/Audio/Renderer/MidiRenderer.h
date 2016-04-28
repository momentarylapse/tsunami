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
	MidiRenderer(Synthesizer *s);
	virtual ~MidiRenderer();

	void __init__(Synthesizer *s);
	virtual void __delete__();

	Synthesizer *getSynthesizer(){ return s; }
	void setSynthesizer(Synthesizer *s);

	virtual int read(BufferBox &buf);
	virtual void reset();

	void feed(const MidiRawData &data);
	void endAllNotes();
	void resetMidiData();

	void setAutoStop(bool auto_stop);

	virtual int getSampleRate();

private:
	Synthesizer *s;
	bool auto_stop;
};

#endif /* SRC_AUDIO_RENDERER_MIDIRENDERER_H_ */
