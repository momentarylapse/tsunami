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

	bool Start(int type, int sample_rate);
	void Stop();
	void Update();

	bool IsCapturing();
	int GetDelay();
	void ResetSync();

	void Accumulate(bool enable);
	void ResetAccumulation();
	int GetSampleCount();

	virtual float GetSampleRate();
	virtual void GetSomeSamples(BufferBox &buf, int num_samples);

	BufferBox current_buffer, buffer;
	MidiData midi;

	int type;
	AudioInputBase *in_cur;
	AudioInputAudio *in_audio;
	AudioInputMidi *in_midi;
	bool running;
};

#endif /* AUDIOINPUT_H_ */
