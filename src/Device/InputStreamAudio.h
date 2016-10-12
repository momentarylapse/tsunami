/*
 * InputStreamAudio.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef INPUTSTREAMAUDIO_H_
#define INPUTSTREAMAUDIO_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include "../Data/Song.h"
#include "../Data/RingBuffer.h"
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

	int _cdecl getDelay();
	void _cdecl resetSync();

	int doCapturing();


	bool _cdecl isCapturing();


	void _cdecl accumulate(bool enable);
	void _cdecl resetAccumulation();
	int _cdecl getSampleCount();

	virtual float _cdecl getSampleRate();
	virtual void _cdecl getSomeSamples(BufferBox &buf, int num_samples);
	virtual int _cdecl getState();

	// delay/sync
	static float getPlaybackDelayConst();
	static void setPlaybackDelayConst(float f);

	// backup
	static string getDefaultBackupFilename();
	static string getBackupFilename();
	static void setBackupFilename(const string &filename);
	static void setTempBackupFilename(const string &filename);
	void setBackupMode(int mode);
	static string temp_backup_filename;
	static string backup_filename;


	void _cdecl setChunkSize(int size);
	int chunk_size;
	void _cdecl setUpdateDt(float dt);
	float update_dt;


	RingBuffer current_buffer;
	BufferBox buffer;

private:

	int sample_rate;
	bool accumulating;
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
	static string cur_backup_filename;
	int backup_mode;

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

#endif /* INPUTSTREAMAUDIO_H_ */
