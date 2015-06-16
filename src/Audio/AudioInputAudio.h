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
#include "AudioInput.h"

struct pa_stream;

#define NUM_CAPTURE_SAMPLES		8192

class AudioInputAudio : public AudioInput
{
public:
	AudioInputAudio(int sample_rate);
	virtual ~AudioInputAudio();

	static Array<string> getDevices();
	static void setFavoriteDevice(const string &device);
	static string getFavoriteDevice();
	virtual void setDevice(const string &device);
	virtual string getChosenDevice();

	virtual bool start();
	virtual void stop();

	virtual int getDelay();
	virtual void resetSync();

	virtual int doCapturing();

	virtual void resetAccumulation();
	virtual int getSampleCount();

	virtual float getSampleRate();
	virtual void getSomeSamples(BufferBox &buf, int num_samples);

	static float getPlaybackDelayConst();
	static void setPlaybackDelayConst(float f);

	static string getDefaultTempFilename();
	static string getTempFilename();
	static void setTempFilename(const string &filename);

	static string temp_filename;

private:

	int num_channels;

	static string favorite_device;
	string chosen_device;

	pa_stream *_stream;

	static bool testError(const string &msg);

	File *temp_file;
	static string cur_temp_filename;

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

	static float playback_delay_const;

	static void input_request_callback(pa_stream *p, size_t nbytes, void *userdata);
};

#endif /* AUDIOINPUTAUDIO_H_ */
