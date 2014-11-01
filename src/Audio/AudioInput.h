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
#include "../View/Helper/PeakMeter.h"

class AudioInputBase;
class AudioInputAudio;
class AudioInputMidi;

class AudioInput : public PeakMeterSource
{
public:
	AudioInput();
	virtual ~AudioInput();

	static const string MESSAGE_CAPTURE;

	bool start(int type, int sample_rate);
	void stop();
	void update();

	bool isCapturing();
	int getDelay();
	void resetSync();

	void accumulate(bool enable);
	void resetAccumulation();
	int getSampleCount();

	virtual float getSampleRate();
	virtual void getSomeSamples(BufferBox &buf, int num_samples);
	virtual int getState();

	BufferBox current_buffer, buffer;
	MidiData midi;

	int type;
	AudioInputBase *in_cur;
	AudioInputAudio *in_audio;
	AudioInputMidi *in_midi;
	bool running;
};

#endif /* AUDIOINPUT_H_ */
