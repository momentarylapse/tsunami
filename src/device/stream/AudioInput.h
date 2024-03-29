/*
 * AudioInput.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_DEVICE_STREAM_AUDIOINPUT_H_
#define SRC_DEVICE_STREAM_AUDIOINPUT_H_

#include "../../lib/base/base.h"
#include "../../lib/base/optional.h"
#include "../../data/audio/RingBuffer.h"
#include "../../module/port/Port.h"
#include "../../module/Module.h"
#include "../../module/ModuleConfiguration.h"

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


class AudioInput : public Module {
	friend class PluginManager;
public:
	AudioInput(Session *session);
	virtual ~AudioInput();

	void _cdecl __init__(Session *session);
	void __delete__() override;

	void _cdecl set_device(Device *device);
	Device* _cdecl get_device();

	void _create_dev();
	void _kill_dev();
	void _pause();
	void _unpause();

	bool _cdecl start();
	void _cdecl stop();


	bool _cdecl is_capturing();

	int _cdecl sample_rate(){ return _sample_rate; }


	RingBuffer buffer;

	class Output : public Port {
	public:
		Output(AudioInput *s);
		int read_audio(AudioBuffer &buf) override;

		AudioInput *stream;
	};

	void _cdecl set_chunk_size(int size);
	int chunk_size;

	int _sample_rate;
	
	base::optional<int64> samples_recorded();

protected:

	DeviceManager *dev_man;

	enum class State {
		NO_DEVICE,
		CAPTURING,
		PAUSED,
	} state;

	int num_channels;


	class Config : public ModuleConfiguration {
	public:
		Device *device;
		void reset() override;
		string auto_conf(const string &name) const override;
	} config;

	ModuleConfiguration* get_config() const override;
	void on_config() override;

	Device *cur_device;
	void update_device();

#if HAS_LIB_PULSEAUDIO
	pa_stream *pulse_stream;
#endif
#if HAS_LIB_PORTAUDIO
	PaStream *portaudio_stream;
#endif


#if HAS_LIB_PULSEAUDIO
	bool _pulse_test_error(const char *msg);
	static void pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata);
	static void pulse_input_notify_callback(pa_stream *p, void *userdata);
	static void pulse_stream_success_callback(pa_stream *s, int success, void *userdata);
	static void pulse_stream_state_callback(pa_stream *s, void *userdata);
#endif
#if HAS_LIB_PORTAUDIO
	static int portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
	                                             unsigned long frames,
	                                             const PaStreamCallbackTimeInfo* timeInfo,
	                                             PaStreamCallbackFlags statusFlags,
	                                             void *userData);
	bool _portaudio_test_error(PaError err, const char *msg);
#endif

public:
	base::optional<int64> command(ModuleCommand cmd, int64 param) override;

};

#endif /* SRC_DEVICE_STREAM_AUDIOINPUT_H_ */
