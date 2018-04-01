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
#include "../Audio/RingBuffer.h"
#include "../Stuff/Observable.h"

class AudioSource;
class DeviceManager;
class Device;
class Thread;
class StreamThread;
class Session;

#if HAS_LIB_PULSEAUDIO
struct pa_stream;
#endif

#if HAS_LIB_PORTAUDIO
typedef void PaStream;
struct PaStreamCallbackTimeInfo;
typedef unsigned long PaStreamCallbackFlags;
typedef int PaError;
#endif

class OutputStream : public Observable<VirtualBase>
{
	friend StreamThread;
public:
	//AudioStream();
	OutputStream(Session *session, AudioSource *r);
	virtual ~OutputStream();

	void _cdecl __init__(Session *session, AudioSource *r);
	virtual void _cdecl __delete__();

	void _create_dev();
	void _kill_dev();

	int api;

	static const string MESSAGE_STATE_CHANGE;
	static const string MESSAGE_READ_END_OF_STREAM;
	static const string MESSAGE_PLAY_END_OF_STREAM;
	static const string MESSAGE_UPDATE;


	void _cdecl stop();
	void _cdecl play();
	void _cdecl pause(bool pause);
	void _cdecl update();

	bool fully_initialized;
	void _start_first_time();
	void _unpause();
	void _pause();

	bool _cdecl isPaused();
	void _cdecl setSource(AudioSource *r);
	void _cdecl setDevice(Device *d);
	int _cdecl getPos();

	void _cdecl clearBuffer();


	// PeakMeterSource
	float _cdecl getSampleRate();

	float _cdecl getVolume();
	void _cdecl setVolume(float _volume);

	void _cdecl setBufferSize(int _size){ buffer_size = _size; }

private:
	void readStream();

	Session *session;

	float volume;
	bool paused;
	int buffer_size;
	float update_dt;
	int hui_runner_id;

	AudioSource *source;
	RingBuffer ring_buf;

	bool keep_thread_running;
	bool reading;
	bool read_more;
	bool read_end_of_stream;
	bool played_end_of_stream;

	int data_samples;

#if HAS_LIB_PULSEAUDIO
	pa_stream *pulse_stream;
	bool _pulse_test_error(const string &msg);
#endif

#if HAS_LIB_PORTAUDIO
	PaStream *portaudio_stream;
	bool _portaudio_test_error(PaError err, const string &msg);
#endif

	int dev_sample_rate;

	DeviceManager *device_manager;
	Device *device;
	bool killed;

	Thread *thread;
	int perf_channel;

#if HAS_LIB_PULSEAUDIO
	static void pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata);
	static void pulse_stream_underflow_callback(pa_stream *s, void *userdata);
	static void pulse_stream_success_callback(pa_stream *s, int success, void *userdata);
#endif

#if HAS_LIB_PORTAUDIO
	static int portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
	                                             unsigned long frames,
	                                             const PaStreamCallbackTimeInfo* timeInfo,
	                                             PaStreamCallbackFlags statusFlags,
	                                             void *userData);
#endif

	void onPlayedEndOfStream();
	void onReadEndOfStream();
};

#endif /* SRC_DEVICE_OUTPUTSTREAM_H_ */
