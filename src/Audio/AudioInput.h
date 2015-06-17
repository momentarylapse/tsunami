/*
 * AudioInput.h
 *
 *  Created on: 22.02.2013
 *      Author: michi
 */

#ifndef AUDIOINPUT_H_
#define AUDIOINPUT_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include "../Data/AudioFile.h"
#include "../Data/RingBuffer.h"
#include "../View/Helper/PeakMeter.h"

class AudioInput : public PeakMeterSource
{
public:
	struct MidiPort
	{
		int client, port;
		string client_name, port_name;
		MidiPort();
	};

	AudioInput(int sample_rate);
	virtual ~AudioInput();

	static const string MESSAGE_CAPTURE;

	virtual bool start() = 0;
	virtual void stop() = 0;

	void _startUpdate();
	void _stopUpdate();
	void update();
	virtual int doCapturing() = 0;

	virtual bool isCapturing();
	virtual int getDelay(){ return 0; }
	virtual void resetSync(){}

	virtual void accumulate(bool enable);
	virtual void resetAccumulation() = 0;
	virtual int getSampleCount() = 0;

	virtual float getSampleRate(){ return sample_rate; }
	virtual void getSomeSamples(BufferBox &buf, int num_samples) = 0;
	virtual int getState();

	virtual void setDevice(const string &dev){}
	virtual string getChosenDevice(){ return ""; }

	virtual MidiPort getCurMidiPort(){ MidiPort p; return p; }
	virtual Array<MidiPort> findMidiPorts(){ Array<MidiPort> a; return a; }
	virtual void connectMidiPort(MidiPort &p){}
	virtual void setPreviewSynthesizer(Synthesizer *s){}

	int sample_rate;
	bool accumulating;
	bool capturing;

	RingBuffer current_buffer;
	BufferBox buffer;
	MidiData midi;
	MidiData current_midi;

	int type;
	bool running;
	int hui_runner_id;
};

#endif /* AUDIOINPUT_H_ */
