/*
 * AudioInput.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef AUDIOINPUT_H_
#define AUDIOINPUT_H_

#include "../lib/file/file.h"
#include "../lib/hui/hui.h"
#include "../Data/AudioFile.h"
#include "../Stuff/PeakMeter.h"


#define NUM_CAPTURE_SAMPLES		8192

class AudioInput : public HuiEventHandler, public PeakMeterSource
{
public:
	AudioInput();
	virtual ~AudioInput();


	bool Capturing, CapturingByDialog, CaptureAddData, CapturePlayback;
	BufferBox CaptureBuf, CapturePreviewBuf;
	float CapturePlaybackDelay;

	Array<string> Device;
	string ChosenDevice;

	void CaptureInit();

	bool CaptureStart(int sample_rate, bool add_data);
	void CaptureStop();
	void AddToCaptureBuf(int a);
	void AddToCapturePreviewBuf(int a);
	int DoCapturing();
	void Update();
	void FindPeaks(int a, float &peak_r, float &peak_l);


	float GetSampleRate();
	BufferBox GetSomeSamples();


	int capture_temp[NUM_CAPTURE_SAMPLES];
	float CaptureLevelR, CaptureLevelL;
	int CaptureSampleRate;
	int CaptureMaxDelay;

	int CaptureCurrentSamples;

	string dev_name;
	void *capture;

};

#endif /* AUDIOINPUT_H_ */
