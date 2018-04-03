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
#include "../Audio/Source/AudioPort.h"
#include "../Plugins/Configurable.h"

class PluginManager;
class Device;
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


class InputStreamAudio : public Configurable
{
	friend class PluginManager;
public:
	InputStreamAudio(Session *session);
	virtual ~InputStreamAudio();

	void _cdecl __init__(Session *session);
	virtual void _cdecl __delete__();

	static const string MESSAGE_CAPTURE;

	void _start_update();
	void _stop_update();
	void update();

	void _cdecl set_device(Device *device);
	Device* _cdecl get_device();

	bool _cdecl start();
	void _cdecl stop();
	void _stop();

	int _cdecl get_delay();
	void _cdecl reset_sync();

	int do_capturing();


	bool _cdecl is_capturing();

	int _cdecl sample_rate(){ return _sample_rate; }

	// delay/sync
	static float get_playback_delay_const();
	static void set_playback_delay_const(float f);


	RingBuffer buffer;

	class Output : public AudioPort
	{
	public:
		Output(InputStreamAudio *s);
		virtual ~Output(){}
		virtual int _cdecl read(AudioBuffer &buf);

		InputStreamAudio *stream;
	};
	Output *out;

	void _cdecl set_backup_mode(int mode);
	int backup_mode;
	File *backup_file;

	void _cdecl set_chunk_size(int size);
	void _cdecl set_update_dt(float dt);
	int chunk_size;
	float update_dt;

	int _sample_rate;

private:

	int api;

	//bool accumulating;
	bool capturing;

	bool running;
	int hui_runner_id;

	int num_channels;

	Device *device;

#if HAS_LIB_PULSEAUDIO
	pa_stream *pulse_stream;
#endif
#if HAS_LIB_PORTAUDIO
	PaStream *portaudio_stream;
#endif

	bool _pulse_test_error(const string &msg);

	struct SyncData
	{
		int num_points;
		long long int delay_sum;
		int samples_in, offset_out;

		void reset();
		void add(int samples);
		int get_delay();
	};
	SyncData sync;

	static float playback_delay_const;

#if HAS_LIB_PULSEAUDIO
	static void pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata);
#endif
#if HAS_LIB_PORTAUDIO
	static int portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
	                                             unsigned long frames,
	                                             const PaStreamCallbackTimeInfo* timeInfo,
	                                             PaStreamCallbackFlags statusFlags,
	                                             void *userData);
#endif

};

#endif /* SRC_DEVICE_INPUTSTREAMAUDIO_H_ */
