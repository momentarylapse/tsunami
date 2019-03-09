/*
 * InputStreamAudio.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_DEVICE_INPUTSTREAMAUDIO_H_
#define SRC_DEVICE_INPUTSTREAMAUDIO_H_

#include "../lib/base/base.h"
#include "../Data/Audio/RingBuffer.h"
#include "../Module/Port/Port.h"
#include "../Module/Module.h"

class PluginManager;
class DeviceManager;
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


class InputStreamAudio : public Module
{
	friend class PluginManager;
public:
	InputStreamAudio(Session *session);
	virtual ~InputStreamAudio();

	void _cdecl __init__(Session *session);
	virtual void _cdecl __delete__();

	void _cdecl set_device(Device *device);
	Device* _cdecl get_device();

	void _create_dev();
	void _kill_dev();
	void _pause();
	void _unpause();

	bool _cdecl start();
	void _cdecl stop();

	int _cdecl get_delay();
	void _cdecl reset_sync();


	bool _cdecl is_capturing();

	int _cdecl sample_rate(){ return _sample_rate; }

	// delay/sync
	static float get_playback_delay_const();
	static void set_playback_delay_const(float f);


	RingBuffer buffer;

	class Output : public Port
	{
	public:
		Output(InputStreamAudio *s);
		virtual ~Output(){}
		int read_audio(AudioBuffer &buf) override;

		InputStreamAudio *stream;
	};
	Output *out;

	void _cdecl set_chunk_size(int size);
	int chunk_size;

	int _sample_rate;

private:

	DeviceManager *dev_man;

	enum class State{
		NO_DEVICE,
		CAPTURING,
		PAUSED,
	} state;

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
	bool _portaudio_test_error(PaError err, const string &msg);
#endif

public:
	int command(ModuleCommand cmd, int param) override;

};

#endif /* SRC_DEVICE_INPUTSTREAMAUDIO_H_ */
