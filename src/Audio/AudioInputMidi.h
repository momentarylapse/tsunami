/*
 * AudioInputMidi.h
 *
 *  Created on: 19.02.2013
 *      Author: michi
 */

#ifndef AUDIOINPUTMIDI_H_
#define AUDIOINPUTMIDI_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include "../Data/AudioFile.h"
#include "AudioInputBase.h"

struct _snd_seq;
struct _snd_seq_port_subscribe;
class AudioStream;
class SynthesizerRenderer;
class Synthesizer;

class AudioInputMidi : public AudioInputBase
{
public:
	AudioInputMidi(MidiData &data);
	virtual ~AudioInputMidi();

	void init();

	virtual bool start(int sample_rate);
	virtual void stop();
	virtual int doCapturing();

	virtual bool isCapturing();

	virtual int getDelay();
	virtual void resetSync();

	virtual void accumulate(bool enable);
	virtual void resetAccumulation();
	virtual int getSampleCount();

	virtual float getSampleRate();
	virtual void getSomeSamples(BufferBox &buf, int num_samples);

	struct MidiPort
	{
		int client, port;
		string client_name, port_name;
		MidiPort();
	};
	Array<MidiPort> findPorts();
	MidiPort getCurMidiPort();
	bool connectTo(MidiPort &p);
	bool unconnect();

	void setPreviewSynthesizer(Synthesizer *s);

private:

	void clearInputQueue();

	MidiData &data;

	_snd_seq *handle;
	_snd_seq_port_subscribe *subs;
	MidiPort cur_midi_port;
	MidiPort no_midi_port;
	int portid;
	int npfd;
	struct pollfd *pfd;
	int sample_rate;
	HuiTimer timer;
	double offset;
	bool capturing;
	bool accumulating;

	AudioStream *preview_stream;
	SynthesizerRenderer *preview_renderer;
};

#endif /* AUDIOINPUTMIDI_H_ */
