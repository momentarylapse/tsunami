/*
 * AudioOutput.h
 *
 *  Created on: 01.11.2014
 *      Author: michi
 */

#ifndef SRC_DEVICE_STREAM_AUDIOOUTPUT_H_
#define SRC_DEVICE_STREAM_AUDIOOUTPUT_H_



#include "../../lib/base/base.h"
#include "../../Data/Audio/RingBuffer.h"
#include "../../Module/Module.h"
#include "../../Module/ModuleConfiguration.h"
#include <atomic>

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

class AudioOutput : public Module {
public:
	AudioOutput(Session *session);
	virtual ~AudioOutput();

	void _cdecl __init__(Session *session);
	void __delete__() override;
	
	static const int DEFAULT_PREBUFFER_SIZE;

	void _create_dev();
	void _kill_dev();

	void reset_state() override;


	void _cdecl stop();
	void _cdecl start();

	void _pause();
	void _unpause();

	bool _cdecl is_playing();

	bool buffer_is_cleared;
	void _fill_prebuffer();

	void _cdecl set_device(Device *d);
	int _cdecl get_available();

	float _cdecl get_volume();
	void _cdecl set_volume(float volume);

	void _cdecl set_prebuffer_size(int size);

	int64 samples_played();

	int get_latency();

private:
	int _read_stream(int buffer_size);

	Port *source;
	RingBuffer ring_buf;

	int prebuffer_size;

	std::atomic<bool> read_end_of_stream;
	std::atomic<bool> played_end_of_stream;

#if HAS_LIB_PULSEAUDIO
	pa_stream *pulse_stream;
	bool _pulse_test_error(const char *msg);
#endif

#if HAS_LIB_PORTAUDIO
	PaStream *portaudio_stream;
	bool _portaudio_test_error(PaError err, const char *msg);
#endif

	int dev_sample_rate;
	int64 samples_requested = 0;
	int64 fake_samples_played = 0;

	DeviceManager *device_manager;


	class Config : public ModuleConfiguration {
	public:
		Device *device;
		float volume;
		void reset() override;
		string auto_conf(const string &name) const override;
	} config;

	ModuleConfiguration* get_config() const override;
	void on_config() override;

	Device *cur_device;
	void update_device();

	enum class State {
		NO_DEVICE,
		DEVICE_WITHOUT_DATA,
		PAUSED,
		PLAYING,
	} state;

	int latency;
	//timeval xxx_prev_time;


	bool feed_stream_output(int frames, float *out);

#if HAS_LIB_PULSEAUDIO
	static void pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata);
	static void pulse_stream_underflow_callback(pa_stream *s, void *userdata);
	static void pulse_stream_success_callback(pa_stream *s, int success, void *userdata);
	static void pulse_stream_state_callback(pa_stream *s, void *userdata);
#endif

#if HAS_LIB_PORTAUDIO
	static int portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
	                                             unsigned long frames,
	                                             const PaStreamCallbackTimeInfo* timeInfo,
	                                             PaStreamCallbackFlags statusFlags,
	                                             void *userData);
#endif

	void on_played_end_of_stream();
	void on_read_end_of_stream();


public:
	int command(ModuleCommand cmd, int param) override;
};

#endif /* SRC_DEVICE_STREAM_AUDIOOUTPUT_H_ */
