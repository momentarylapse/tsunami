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
#include "../Data/RingBuffer.h"
#include "AudioInputBase.h"

typedef void PaStream;

#define NUM_CAPTURE_SAMPLES		8192

class AudioInputAudio : public AudioInputBase
{
public:
	AudioInputAudio(BufferBox &buf, RingBuffer &cur_buf);
	virtual ~AudioInputAudio();

	string chosen_device;
	string temp_filename;

	Array<string> getDevices();
	void setDevice(const string &device);

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

	BufferBox &accumulation_buffer;
	RingBuffer &current_buffer;
	int num_channels;
private:
	bool accumulating;

	int capture_temp[NUM_CAPTURE_SAMPLES];

	int pa_device_no;
	PaStream *pa_stream;

	bool testError(const string &msg);
	int last_error;

	CFile *temp_file;
	string cur_temp_filename;

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

	bool capturing;
	int sample_rate;
	float playback_delay_const;
};

#endif /* AUDIOINPUTAUDIO_H_ */
