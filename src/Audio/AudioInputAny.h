/*
 * AudioInputAny.h
 *
 *  Created on: 16.08.2015
 *      Author: michi
 */

#ifndef SRC_AUDIO_AUDIOINPUTANY_H_
#define SRC_AUDIO_AUDIOINPUTANY_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include "../Data/RingBuffer.h"
#include "../Data/Song.h"
#include "../View/Helper/PeakMeter.h"
#include "AudioInputMidi.h"

class AudioInputAudio;
class AudioInputMidi;

class AudioInputAny : public PeakMeterSource, public Observer
{
public:

	AudioInputAny(int sample_rate);
	virtual ~AudioInputAny();

	static const string MESSAGE_CAPTURE;

	void setType(int type);

	bool start();
	void stop();

	bool isCapturing();
	int getDelay(){ return 0; }
	void resetSync(){}

	void accumulate(bool enable);
	void resetAccumulation();
	int getSampleCount();

	virtual float getSampleRate(){ return sample_rate; }
	virtual void getSomeSamples(BufferBox &buf, int num_samples);
	virtual int getState();

	void setDevice(const string &dev);
	string getChosenDevice();

	void setSaveMode(bool enabled);

	AudioInputMidi::MidiPort getCurMidiPort();
	Array<AudioInputMidi::MidiPort> findMidiPorts();
	bool connectMidiPort(AudioInputMidi::MidiPort &p);
	void setPreviewSynthesizer(Synthesizer *s);

	virtual void onUpdate(Observable *o, const string &message);

	int sample_rate;

	RingBuffer *current_buffer;
	BufferBox *buffer;
	MidiData *midi;
	MidiData *current_midi;

	int type;
	AudioInputAudio *input_audio;
	AudioInputMidi *input_midi;
	bool save_mode;
};

#endif /* SRC_AUDIO_AUDIOINPUTANY_H_ */
