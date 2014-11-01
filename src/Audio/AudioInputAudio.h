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
	string TempFilename;

	void init();

	virtual bool start(int sample_rate);
	virtual void stop();

	virtual bool isCapturing();
	virtual int getDelay();
	virtual void resetSync();

	virtual int doCapturing();

	virtual void accumulate(bool enable);
	virtual void resetAccumulation();
	virtual int getSampleCount();

	virtual float getSampleRate();
	virtual void getSomeSamples(BufferBox &buf, int num_samples);

	float getPlaybackDelayConst();
	void setPlaybackDelayConst(float f);

	string getDefaultTempFilename();
	string getTempFilename();
	void setTempFilename(const string &filename);

private:
	BufferBox &AccumulationBuffer, &CurrentBuffer;
	bool accumulating;

	int capture_temp[NUM_CAPTURE_SAMPLES];
	ALCdevice_struct *capture;

	CFile *temp_file;
	string cur_temp_filename;

	string dev_name;

	struct SyncData
	{
		int num_points;
		long long int delay_sum;
		int samples_in, offset_out;

		void reset();
		void add(int samples);
		int getDelay();
	};
	SyncData sync;

	bool Capturing;
	int SampleRate;
	float PlaybackDelayConst;
};

#endif /* AUDIOINPUTAUDIO_H_ */
