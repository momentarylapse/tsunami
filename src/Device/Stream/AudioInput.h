/*
 * AudioInput.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_DEVICE_STREAM_AUDIOINPUT_H_
#define SRC_DEVICE_STREAM_AUDIOINPUT_H_

#include "../../lib/base/base.h"
#include "../../Data/Audio/RingBuffer.h"
#include "../../Module/Port/Port.h"
#include "../../Module/Module.h"
#include "../../Module/ModuleConfiguration.h"

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
	virtual void _cdecl __delete__();

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
	Output *out;

	void _cdecl set_chunk_size(int size);
	int chunk_size;

	int _sample_rate;
	
	int64 samples_recorded();

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

	bool _pulse_test_error(const string &msg);

#if HAS_LIB_PULSEAUDIO
	static void pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata);
	static void pulse_input_notify_callback(pa_stream *p, void *userdata);
	static void pulse_stream_success_callback(pa_stream *s, int success, void *userdata);
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

#endif /* SRC_DEVICE_STREAM_AUDIOINPUT_H_ */
