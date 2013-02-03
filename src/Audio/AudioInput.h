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


#define NUM_CAPTURE_SAMPLES		8192

class AudioInput : public HuiEventHandler, public PeakMeterSource
{
public:
	AudioInput();
	virtual ~AudioInput();


	bool Capturing, CapturingByDialog, CaptureAddData, CapturePlayback;
	BufferBox CaptureBuf, CapturePreviewBuf;
	float CapturePlaybackDelayConst;
	int CaptureDelay;

	Array<string> Device;
	string ChosenDevice;

	void Init();

	bool Start(int sample_rate, bool add_data);
	void Stop();
	void AddToCaptureBuf(int a);
	void AddToCapturePreviewBuf(int a);
	int DoCapturing();
	void Update();


	float GetSampleRate();
	BufferBox GetSomeSamples(int num_samples);


	int capture_temp[NUM_CAPTURE_SAMPLES];
	int CaptureSampleRate;
	int CaptureMaxDelay;

	int CaptureCurrentSamples;

	string dev_name;
	void *capture;

private:
	int num_delay_points;
	long long int delay_sum;

};

#endif /* AUDIOINPUT_H_ */
