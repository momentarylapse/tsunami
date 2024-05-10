/*
 * AudioOutput.cpp
 *
 *  Created on: 01.11.2014
 *      Author: michi
 */

#include "AudioOutput.h"
#include "../../Session.h"
#include "../../data/base.h"
#include "../../module/port/Port.h"
#include "../Device.h"
#include "../DeviceManager.h"
#include "../../lib/kaba/lib/extern.h"
#include "../../plugins/PluginManager.h"
#include "../AudioOutputStream.h"
#include "../pulse/AudioOutputStreamPulse.h"
#include "../port/AudioOutputStreamPort.h"

namespace kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}

#if HAS_LIB_PULSEAUDIO
#include <pulse/pulseaudio.h>
#endif

#if HAS_LIB_PORTAUDIO
#include <portaudio.h>
#endif

static const bool STREAM_WARNINGS = true;

const int AudioOutput::DEFAULT_PREBUFFER_SIZE = 4096;


namespace os {
	extern void require_main_thread(const string &msg);
}


// return: underrun?
bool AudioOutput::feed_stream_output(int frames_request, float *out) {

	//printf("feed %d\n", frames_request);
	int available = ring_buf.available();
	int frames = min(frames_request, available);
	samples_requested += frames_request;


	//printf("av=%d r=%d reos=%d\n", available, stream->reading.load(), stream->read_end_of_stream.load());

	// read 2x in case of wrap-around
	int done = 0;
	for (int n=0; (n<2) and (done < frames); n++) {
		AudioBuffer b;
		ring_buf.read_ref(b, frames - done);
		b.interleave(out, device_manager->get_output_volume() * config.volume);
		ring_buf.read_ref_done(b);
		out += b.length * 2;
		done += b.length;
	}
	done = frames;


	// underflow? -> add silence
	if (available < frames_request) {
		if (!read_end_of_stream and !buffer_is_cleared)
			if (STREAM_WARNINGS)
				printf("< underflow  %d < %d\n", available, frames_request);
		// output silence...
		fake_samples_played += frames_request - done;
		for (int i=done; i<frames_request; i++) {
			*out ++ = 0;
			*out ++ = 0;
		}
		return true;
	}

	return false;
}



void AudioOutput::Config::reset() {
	device = _module->session->device_manager->choose_device(DeviceType::AUDIO_OUTPUT);
	volume = 1;
}

string AudioOutput::Config::auto_conf(const string &name) const {
	if (name == "volume")
		return "0:1:0.1:100:%";
	if (name == "device")
		return "audio:output";
	return "";
}


AudioOutput::AudioOutput(Session *_session) :
	Module(ModuleCategory::STREAM, "AudioOutput"),
	ring_buf(1048576) {
	state = State::UNPREPARED_NO_DEVICE_NO_DATA;
	session = _session;

	config.volume = 1;
	prebuffer_size = hui::config.get_int("Output.BufferSize", DEFAULT_PREBUFFER_SIZE);

	auto *device_pointer_class = session->plugin_manager->get_class("Device*");
	device_manager = session->device_manager;
	auto _class = session->plugin_manager->get_class("AudioOutputConfig");//new kaba::Class("Config", sizeof(config), nullptr, nullptr);
	if (_class->elements.num == 0) {
		kaba::add_class(_class);
		kaba::class_add_element("volume", kaba::TypeFloat32, &Config::volume);
		kaba::class_add_element("device", device_pointer_class, &Config::device);
		_class->_vtable_location_target_ = kaba::get_vtable(&config);
	}
	config.kaba_class = _class;

	dev_sample_rate = -1;
	cur_device = nullptr;

	read_end_of_stream = false;
	played_end_of_stream = false;

	buffer_is_cleared = true;
	latency = 0;
}

AudioOutput::~AudioOutput() {
	_pause();
	_kill_dev();
}

void AudioOutput::__init__(Session *s) {
	new(this) AudioOutput(s);
}

void AudioOutput::__delete__() {
	this->AudioOutput::~AudioOutput();
}

void AudioOutput::_create_dev() {
	if (has_device())
		return;
	if (!device_manager->audio_api_initialized())
		return;
	session->debug("out", "create device");
	dev_sample_rate = session->sample_rate();
	device_manager->lock();

#if HAS_LIB_PULSEAUDIO
	if (device_manager->audio_api == DeviceManager::ApiType::PULSE) {
		stream = new AudioOutputStreamPulse(session, cur_device, [this] (float* out, int frames) {
			bool out_of_data = feed_stream_output(frames, out);
			return out_of_data;
		}, [this] {
			if (read_end_of_stream and !played_end_of_stream) {
				//printf("end of data...\n");
				played_end_of_stream = true;
				hui::run_later(0.001f, [this]{ on_played_end_of_stream(); }); // TODO prevent abort before playback really finished
			}
			//pa_threaded_mainloop_signal(stream->device_manager->pulse_mainloop, 0);
		});
		if (stream->error) {
			delete stream;
			stream = nullptr;
			stop();
			return;
		}
	}
#endif

#if HAS_LIB_PORTAUDIO
	if (device_manager->audio_api == DeviceManager::ApiType::PORTAUDIO) {
		stream = new AudioOutputStreamPort(session, cur_device, [this] (float* out, int frames) {
			bool out_of_data = feed_stream_output(frames, out);
			return out_of_data;
		}, [this] {
			if (read_end_of_stream and !played_end_of_stream) {
				//printf("end of data...\n");
				played_end_of_stream = true;
				hui::run_later(0.001f, [this]{ on_played_end_of_stream(); }); // TODO prevent abort before playback really finished
			}
			//pa_threaded_mainloop_signal(stream->device_manager->pulse_mainloop, 0);
		});
		if (stream->error) {
			delete stream;
			stream = nullptr;
			stop();
			return;
		}
	}
#endif
	device_manager->unlock();
	if (state == State::UNPREPARED_NO_DEVICE_NO_DATA)
		_set_state(State::UNPREPARED_NO_DATA);
	else if (state == State::UNPREPARED_NO_DEVICE)
		_set_state(State::PAUSED);
}

void AudioOutput::_kill_dev() {
	if (!has_device())
		return;
	session->debug("out", "kill device");

	if (stream) {
		delete stream;
		stream = nullptr;
	}

	_clear_data_state();
	_set_state(State::UNPREPARED_NO_DEVICE_NO_DATA);
}

void AudioOutput::_clear_data_state() {
	ring_buf.clear();
	buffer_is_cleared = true;
	read_end_of_stream = false;
	played_end_of_stream = false;
	samples_requested = 0;
	fake_samples_played = 0;
}


void AudioOutput::stop() {
	os::require_main_thread("out.stop");
	_pause();
}

void AudioOutput::_pause() {
	if (state != State::PLAYING)
		return;
	session->debug("out", "pause");

	if (stream)
		stream->pause();

	_set_state(State::PAUSED);
}

void AudioOutput::_set_state(State s) {
	state = s;
	out_state_changed.notify();
}

void AudioOutput::_unpause() {
	if (state != State::PAUSED)
		return;
	session->debug("out", "unpause");

	read_end_of_stream = false;
	played_end_of_stream = false;

	if (stream)
		stream->unpause();

	_set_state(State::PLAYING);
}

int AudioOutput::_read_stream(int buffer_size) {
	if (!in.source)
		return 0;

	int size = 0;
	AudioBuffer b;
	ring_buf.write_ref(b, buffer_size);

	// read data
	size = in.source->read_audio(b);

	if (size == NOT_ENOUGH_DATA) {
		//printf(" -> no data\n");
		// keep trying...
		ring_buf.write_ref_cancel(b);
		return size;
	}

	// out of data?
	if (size == END_OF_STREAM) {
		//printf(" -> end  STREAM\n");
		read_end_of_stream = true;
		hui::run_later(0.001f,  [this] { on_read_end_of_stream(); });
		ring_buf.write_ref_cancel(b);
		return size;
	}

	// add to queue
	b.length = size;
	ring_buf.write_ref_done(b);
	buffer_is_cleared = false;
//	printf(" -> %d of %d\n", size, buffer_size);
	return size;
}

void AudioOutput::set_device(Device *d) {
	config.device = d;
	on_config();
	out_changed.notify();
}

#include "../../lib/os/time.h"

void AudioOutput::start() {
	os::sleep(0.1f);
	os::require_main_thread("out.start");

	if (!has_data())
		_fill_prebuffer();

	if (!has_device())
		_create_dev();

	_unpause();
}

void AudioOutput::_fill_prebuffer() {
	if (has_data())
		return;
	session->debug("out", "prebuf");

	// we need some data in the buffer...
	_read_stream(prebuffer_size);

#if HAS_LIB_PULSEAUDIO
	if (device_manager->audio_api == DeviceManager::ApiType::PULSE) {
		if (!stream)
			return;

		/*device_manager->lock();
		pa_operation *op = pa_stream_prebuf(pulse_stream, &pulse_stream_success_callback, this);
		device_manager->unlock();
		_pulse_start_op(op, "pa_stream_prebuf");*/

		/*pa_operation *op = pa_stream_trigger(pulse_stream, &pulse_stream_success_callback, nullptr);
		_pulse_test_error("pa_stream_trigger");
		pa_wait_op(session, op);*/
	}
#endif

#if HAS_LIB_PORTAUDIO
	if (device_manager->audio_api == DeviceManager::ApiType::PORTAUDIO) {
		if (!stream)
			return;
		//device_manager->lock();
		//PaError err = Pa_StartStream(portaudio_stream);
		//_portaudio_test_error(err, "Pa_StartStream");
		//device_manager->unlock();
	}
#endif

	if (state == State::UNPREPARED_NO_DEVICE_NO_DATA)
		_set_state(State::UNPREPARED_NO_DEVICE);
	else if (state == State::UNPREPARED_NO_DATA)
		_set_state(State::PAUSED);
}

int AudioOutput::get_available() {
	return ring_buf.available();
}

float AudioOutput::get_volume() {
	return config.volume;
}

void AudioOutput::set_volume(float _volume) {
	config.volume = _volume;
	out_state_changed.notify();
	out_changed.notify();
}

void AudioOutput::set_prebuffer_size(int size) {
	prebuffer_size = size;
}



void AudioOutput::update_device() {
	auto old_state = state;
	if (state == State::PLAYING)
		_pause();
	if (has_device())
		_kill_dev();

	cur_device = config.device;

	if (old_state == State::PLAYING)
		start();
}

void AudioOutput::on_config() {
	if (cur_device != config.device)
		update_device();
}

void AudioOutput::on_played_end_of_stream() {
	//printf("---------ON PLAY END OF STREAM\n");
	out_play_end_of_stream.notify();
}

void AudioOutput::on_read_end_of_stream() {
	//printf("---------ON READ END OF STREAM\n");
	out_read_end_of_stream.notify();
}

void AudioOutput::reset_state() {
	if (state == State::UNPREPARED_NO_DEVICE_NO_DATA)
		return;
	if (state == State::PLAYING)
		_pause();
	os::require_main_thread("out.reset");

	if (state == State::PAUSED) {
#if HAS_LIB_PULSEAUDIO
		if (device_manager->audio_api == DeviceManager::ApiType::PULSE) {
			session->debug("out", "flush");
			if (stream)
				samples_offset_since_reset += stream->flush(samples_offset_since_reset, samples_requested);
		}
#endif
		_set_state(State::UNPREPARED_NO_DATA);
	}

	_clear_data_state();
}

base::optional<int64> AudioOutput::command(ModuleCommand cmd, int64 param) {
	if (cmd == ModuleCommand::START) {
		start();
		return 0;
	} else if (cmd == ModuleCommand::STOP) {
		stop();
		return 0;
	} else if (cmd == ModuleCommand::PREPARE_START) {
		if (!has_data())
			_fill_prebuffer();
		if (!has_device())
			_create_dev();
		return 0;
	} else if (cmd == ModuleCommand::SUCK) {
		if (ring_buf.available() >= prebuffer_size)
			return 0;
		//printf("suck %d    av %d\n", param, ring_buf.available());
		return (int64)_read_stream((int)param);
	} else if (cmd == ModuleCommand::SAMPLE_COUNT_MODE) {
		return (int64)SampleCountMode::CONSUMER;
	} else if (cmd == ModuleCommand::GET_SAMPLE_COUNT) {
		auto s = estimate_samples_played();
		if (s)
			return *s;
		return 0;
	}
	return base::None;
}

bool AudioOutput::is_playing() {
	return (state == State::PLAYING) or (state == State::PAUSED);
}

bool AudioOutput::has_data() const {
	return state != State::UNPREPARED_NO_DATA and state != State::UNPREPARED_NO_DEVICE_NO_DATA;
}

bool AudioOutput::has_device() const {
	return state != State::UNPREPARED_NO_DEVICE and state != State::UNPREPARED_NO_DEVICE_NO_DATA;
}

base::optional<int> AudioOutput::get_latency() {
	return base::None;
	//return latency;
}

// (roughly) how many samples might the sound card have played since the last reset?
base::optional<int64> AudioOutput::estimate_samples_played() {
	if (!has_device())
		return base::None;

	if (stream)
		return stream->estimate_samples_played(samples_offset_since_reset, samples_requested);
	return samples_requested;

#if HAS_LIB_PORTAUDIO
	if (device_manager->audio_api == DeviceManager::ApiType::PORTAUDIO) {
	//	always returning 0???
	//	PaTime t = Pa_GetStreamTime(portaudio_stream);
	//	return (double)t / session->sample_rate() - fake_samples_played;
		return samples_requested;
	}
#endif
	return base::None;
}

// requested by audio library (since last reset_state())
int64 AudioOutput::get_samples_requested() const {
	return samples_requested;
}

ModuleConfiguration *AudioOutput::get_config() const {
	return (ModuleConfiguration*)&config;
}
