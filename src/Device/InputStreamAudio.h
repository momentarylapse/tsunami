/*
 * InputStreamAudio.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_DEVICE_INPUTSTREAMAUDIO_H_
#define SRC_DEVICE_INPUTSTREAMAUDIO_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include "../Data/Song.h"
#include "../Audio/RingBuffer.h"
#include "../Audio/Source/AudioSource.h"
#include "../View/Helper/PeakMeter.h"
#include "config.h"

class PluginManager;
class Device;

#ifdef DEVICE_PULSEAUDIO
struct pa_stream;
#endif

class InputStreamAudio : public PeakMeterSource
{
	friend class PluginManager;
public:
	InputStreamAudio(int sample_rate);
	virtual ~InputStreamAudio();

	void _cdecl __init__(int sample_rate);
	virtual void _cdecl __delete__();

	static const string MESSAGE_CAPTURE;

	void _startUpdate();
	void _stopUpdate();
	void update();

	void _cdecl setDevice(Device *device);
	Device* _cdecl getDevice();

	bool _cdecl start();
	void _cdecl stop();
	void _stop();

	int _cdecl getDelay();
	void _cdecl resetSync();

	int doCapturing();


	bool _cdecl isCapturing();

	virtual float _cdecl getSampleRate(){ return sample_rate; }
	virtual void _cdecl getSomeSamples(AudioBuffer &buf, int num_samples);
	virtual int _cdecl getState();

	// delay/sync
	static float getPlaybackDelayConst();
	static void setPlaybackDelayConst(float f);


	RingBuffer buffer;

	class Source : public AudioSource
	{
	public:
		virtual int _cdecl read(AudioBuffer &buf);
		virtual int _cdecl getSampleRate();

		InputStreamAudio *stream;
	};
	Source *source;

	void _cdecl setBackupMode(int mode);
	int backup_mode;

	void _cdecl setChunkSize(int size);
	void _cdecl setUpdateDt(float dt);
	int chunk_size;
	float update_dt;

	int sample_rate;

private:

	//bool accumulating;
	bool capturing;

	bool running;
	int hui_runner_id;

	int num_channels;

	Device *device;

#ifdef DEVICE_PULSEAUDIO
	pa_stream *_stream;
#endif

	static bool testError(const string &msg);

	File *backup_file;

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

#ifdef DEVICE_PULSEAUDIO
	static void input_request_callback(pa_stream *p, size_t nbytes, void *userdata);
#endif
};

#endif /* SRC_DEVICE_INPUTSTREAMAUDIO_H_ */
