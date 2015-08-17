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
#include "../View/Helper/PeakMeter.h"

struct pa_stream;

class AudioInputAudio : public PeakMeterSource
{
public:
	AudioInputAudio(int sample_rate);
	virtual ~AudioInputAudio();

	static const string MESSAGE_CAPTURE;

	void _startUpdate();
	void _stopUpdate();
	void update();

	static Array<string> getDevices();
	static void setFavoriteDevice(const string &device);
	static string getFavoriteDevice();
	void setDevice(const string &device);
	string getChosenDevice();

	bool start();
	void stop();

	int getDelay();
	void resetSync();

	int doCapturing();


	bool isCapturing();


	void accumulate(bool enable);
	void resetAccumulation();
	int getSampleCount();

	virtual float getSampleRate();
	virtual void getSomeSamples(BufferBox &buf, int num_samples);
	virtual int getState();

	static float getPlaybackDelayConst();
	static void setPlaybackDelayConst(float f);

	static string getDefaultTempFilename();
	static string getTempFilename();
	static void setTempFilename(const string &filename);

	static string temp_filename;


	RingBuffer current_buffer;
	BufferBox buffer;

private:

	int sample_rate;
	bool accumulating;
	bool capturing;

	bool running;
	int hui_runner_id;

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
