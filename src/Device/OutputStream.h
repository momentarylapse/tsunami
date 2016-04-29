/*
 * OutputStream.h
 *
 *  Created on: 01.11.2014
 *      Author: michi
 */

#ifndef SRC_DEVICE_OUTPUTSTREAM_H_
#define SRC_DEVICE_OUTPUTSTREAM_H_



#include "../lib/base/base.h"
#include "../lib/hui/hui.h"
#include "../Data/Song.h"
#include "../Data/RingBuffer.h"
#include "../View/Helper/PeakMeter.h"

class AudioRenderer;
class DeviceManager;
class Device;
class Thread;
class StreamThread;
struct pa_stream;

class OutputStream : public PeakMeterSource
{
	friend StreamThread;
public:
	//AudioStream();
	OutputStream(AudioRenderer *r);
	virtual ~OutputStream();

	void _cdecl __init__(AudioRenderer *r);
	virtual void _cdecl __delete__();

	void create_dev();
	void kill_dev();
	void kill();


	static const string MESSAGE_STATE_CHANGE;
	static const string MESSAGE_UPDATE;


	void _cdecl stop();
	void _cdecl play();
	void _cdecl pause();
	void _cdecl update();

	bool _cdecl isPlaying();
	bool _cdecl isPaused();
	int _cdecl getState();
	void _cdecl setSource(AudioRenderer *r);
	AudioRenderer *getSource(){ return renderer; }
	void _cdecl setDevice(Device *d);
	int getPos();
	bool getPosSafe(int &pos);
	void flushBuffers();
	void seek(int pos);

	float getSampleRate();
	void getSomeSamples(BufferBox &buf, int num_samples);

	float getVolume();
	void setVolume(float _volume);

	void setBufferSize(int _size){ buffer_size = _size; }

private:
	bool testError(const string &msg);
	void stream();


	float volume;
	bool playing;
	bool paused;
	int buffer_size;
	float update_dt;
	int hui_runner_id;

	AudioRenderer *renderer;
	RingBuffer ring_buf;

	bool reading;
	bool read_more;
	bool end_of_data;

	int data_samples;

	pa_stream *_stream;
	int dev_sample_rate;
	long long cur_pos;

	DeviceManager *device_manager;
	Device *device;
	bool killed;

	Thread *thread;
	float cpu_usage;

	static void stream_request_callback(pa_stream *p, size_t nbytes, void *userdata);
	static void stream_underflow_callback(pa_stream *s, void *userdata);
	static void stream_success_callback(pa_stream *s, int success, void *userdata);
};

#endif /* SRC_DEVICE_OUTPUTSTREAM_H_ */
