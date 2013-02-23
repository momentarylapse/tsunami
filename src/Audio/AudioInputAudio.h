/*
 * AudioInputAudio.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef AUDIOINPUTAUDIO_H_
#define AUDIOINPUTAUDIO_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include "../Data/AudioFile.h"
#include "AudioInputBase.h"

struct ALCdevice_struct;


#define NUM_CAPTURE_SAMPLES		8192

class AudioInputAudio : public AudioInputBase
{
public:
	AudioInputAudio(BufferBox &buf, BufferBox &cur_buf);
	virtual ~AudioInputAudio();

	Array<string> Device;
	string ChosenDevice;

	void Init();

	virtual bool Start(int sample_rate);
	virtual void Stop();

	virtual bool IsCapturing();
	virtual int GetDelay();
	virtual void ResetSync();

	virtual int DoCapturing();

	virtual void Accumulate(bool enable);
	virtual void ResetAccumulation();
	virtual int GetSampleCount();

	virtual float GetSampleRate();
	virtual BufferBox GetSomeSamples(int num_samples);

	float GetPlaybackDelayConst();
	void SetPlaybackDelayConst(float f);

private:
	BufferBox &AccumulationBuffer, &CurrentBuffer;
	bool accumulate;

	int capture_temp[NUM_CAPTURE_SAMPLES];
	ALCdevice_struct *capture;

	string dev_name;

	struct SyncData
	{
		int num_points;
		long long int delay_sum;
		int samples_in, offset_out;

		void Reset();
		void Add(int samples);
		int GetDelay();
	};
	SyncData sync;

	bool Capturing;
	int SampleRate;
	float PlaybackDelayConst;
};

#endif /* AUDIOINPUTAUDIO_H_ */
