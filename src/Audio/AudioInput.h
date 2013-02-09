/*
 * AudioInput.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef AUDIOINPUT_H_
#define AUDIOINPUT_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include "../Data/AudioFile.h"
#include "../View/Helper/PeakMeter.h"

struct ALCdevice_struct;


#define NUM_CAPTURE_SAMPLES		8192

class AudioInput : public HuiEventHandler, public PeakMeterSource
{
public:
	AudioInput();
	virtual ~AudioInput();


	BufferBox CurrentBuffer;

	Array<string> Device;
	string ChosenDevice;

	void Init();

	bool Start(int sample_rate);
	void Stop();
	void Update();

	bool IsCapturing();
	int GetDelay();
	void ResetSync();


	virtual float GetSampleRate();
	virtual BufferBox GetSomeSamples(int num_samples);

	float GetPlaybackDelayConst();
	void SetPlaybackDelayConst(float f);

private:
	int DoCapturing();

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

#endif /* AUDIOINPUT_H_ */
