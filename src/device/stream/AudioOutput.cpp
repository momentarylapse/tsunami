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

#if HAS_LIB_PULSEAUDIO

extern void pulse_wait_op(Session*, pa_operation*); // -> DeviceManager.cpp
//extern void pulse_ignore_op(Session*, pa_operation*);


// DeviceManager needs to be locked!
bool pulse_wait_stream_ready(pa_stream *s, DeviceManager *dm) {
	//msg_write("wait stream ready");
	int n = 0;
	while (pa_stream_get_state(s) != PA_STREAM_READY) {
		//printf(".\n");
		//pa_mainloop_iterate(m, 1, NULL);
		//hui::Sleep(0.01f);
		pa_threaded_mainloop_wait(dm->pulse_mainloop);
		n ++;
		if (n >= 1000)
			return false;
		if (pa_stream_get_state(s) == PA_STREAM_FAILED)
			return false;
	}
	//msg_write("ok");
	return true;
}

int nnn = 0;
int xxx_total_read = 0;


void AudioOutput::pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata) {
	//printf("output request %d\n", (int)nbytes/8);
	auto stream = static_cast<AudioOutput*>(userdata);

	if (nbytes == 0)
		return;

	void *data = nullptr;
	int r = pa_stream_begin_write(p, &data, &nbytes);
	if (r != 0) {
		stream->_pulse_test_error("pa_stream_begin_write");
		return;
	}
	//printf("%d  %p  %d\n", r, data, (int)nbytes);

	int frames = nbytes / 8;
	float *out = static_cast<float*>(data);

	bool out_of_data = stream->feed_stream_output(frames, out);

	if (pa_stream_write(p, data, nbytes, nullptr, 0, (pa_seek_mode_t)PA_SEEK_RELATIVE) != 0)
		stream->_pulse_test_error("pa_stream_write");

	if (out_of_data and stream->read_end_of_stream and !stream->played_end_of_stream) {
		//printf("end of data...\n");
		stream->played_end_of_stream = true;
		hui::run_later(0.001f, [stream]{ stream->on_played_end_of_stream(); }); // TODO prevent abort before playback really finished
	}
	//pa_threaded_mainloop_signal(stream->device_manager->pulse_mainloop, 0);
}

void AudioOutput::pulse_stream_success_callback(pa_stream *s, int success, void *userdata) {
	auto stream = static_cast<AudioOutput*>(userdata);
	//msg_write("--success");
	pa_threaded_mainloop_signal(stream->device_manager->pulse_mainloop, 0);
}

void AudioOutput::pulse_stream_state_callback(pa_stream *s, void *userdata) {
	auto stream = static_cast<AudioOutput*>(userdata);
	//printf("--state\n");
	pa_threaded_mainloop_signal(stream->device_manager->pulse_mainloop, 0);
}

void AudioOutput::pulse_stream_underflow_callback(pa_stream *s, void *userdata) {
	//auto stream = static_cast<AudioOutput*>(userdata);
	//stream->session->w("pulse: underflow\n");
	if (STREAM_WARNINGS)
		printf("pulse: underflow\n");
	//pa_threaded_mainloop_signal(stream->device_manager->pulse_mainloop, 0);
}

#endif


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

#if HAS_LIB_PORTAUDIO

int AudioOutput::portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
                                                    unsigned long frames,
                                                    const PaStreamCallbackTimeInfo* timeInfo,
                                                    PaStreamCallbackFlags statusFlags,
                                                    void *userData) {
//	printf("request %d\n", (int)frames);
	auto stream = static_cast<AudioOutput*>(userData);

	auto out = static_cast<float*>(outputBuffer);
	(void) inputBuffer; /* Prevent unused variable warning. */

	bool out_of_data = stream->feed_stream_output((int)frames, out);

	if (out_of_data and stream->read_end_of_stream and !stream->played_end_of_stream) {
		//printf("XXX end of data...\n");
		stream->played_end_of_stream = true;
		hui::run_later(0.001f, [stream]{ stream->on_played_end_of_stream(); }); // TODO prevent abort before playback really finished
		//printf("/XXX end of data...\n");
		//return paComplete;
	}
	return paContinue;
}

#endif


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

#if HAS_LIB_PULSEAUDIO
	pulse_stream = nullptr;
#endif
#if HAS_LIB_PORTAUDIO
	portaudio_stream = nullptr;
#endif
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
		pa_sample_spec ss;
		ss.rate = dev_sample_rate;
		ss.channels = 2;
		ss.format = PA_SAMPLE_FLOAT32NE;
		//ss.format = PA_SAMPLE_S16NE;
		pulse_stream = pa_stream_new(device_manager->pulse_context, "stream", &ss, nullptr);
		if (!pulse_stream)
			_pulse_test_error("pa_stream_new");

		pa_stream_set_state_callback(pulse_stream, &pulse_stream_state_callback, this);
		pa_stream_set_write_callback(pulse_stream, &pulse_stream_request_callback, this);
		pa_stream_set_underflow_callback(pulse_stream, &pulse_stream_underflow_callback, this);

		pa_buffer_attr attr_out;
		attr_out.fragsize = -1; // recording only
		attr_out.maxlength = hui::config.get_int("Output.Pulseaudio.maxlength", -1);
		attr_out.minreq = hui::config.get_int("Output.Pulseaudio.minreq", 1024);
		attr_out.tlength = hui::config.get_int("Output.Pulseaudio.tlength", 1024);
		attr_out.prebuf = hui::config.get_int("Output.Pulseaudio.prebuf", 0); // don't prebuffer
		// prebuf = 0 also prevents pausing during buffer underruns

		const char *dev = nullptr;
		if (!cur_device->is_default())
			dev = cur_device->internal_name.c_str();
		auto flags = (pa_stream_flags_t)(PA_STREAM_START_CORKED|PA_STREAM_AUTO_TIMING_UPDATE|PA_STREAM_INTERPOLATE_TIMING);

		if (pa_stream_connect_playback(pulse_stream, dev, &attr_out, flags, nullptr, nullptr) != 0)
			_pulse_test_error("pa_stream_connect_playback");


		if (!pulse_wait_stream_ready(pulse_stream, device_manager)) {
			session->w("retry");

			// retry with default device
			if (pa_stream_connect_playback(pulse_stream, nullptr, &attr_out, flags, nullptr, nullptr) != 0)
				_pulse_test_error("pa_stream_connect_playback");

			if (!pulse_wait_stream_ready(pulse_stream, device_manager)) {
				device_manager->unlock();
				// still no luck... give up
				session->e("pulse_wait_stream_ready");
//				pa_threaded_mainloop_unlock(device_manager->pulse_mainloop);
				stop();
				return;
			}
		}
	}
#endif

#if HAS_LIB_PORTAUDIO
	if (device_manager->audio_api == DeviceManager::ApiType::PORTAUDIO) {


		int chunk_size = hui::config.get_int("Output.Portaudio.chunk-size", 256);
		//256*4; // paFramesPerBufferUnspecified
		if (cur_device->is_default()) {
			PaError err = Pa_OpenDefaultStream(&portaudio_stream, 0, 2, paFloat32, dev_sample_rate, chunk_size,
					&portaudio_stream_request_callback, this);
			_portaudio_test_error(err, "Pa_OpenDefaultStream");
		} else {
			PaStreamParameters params;
			params.channelCount = 2;
			params.sampleFormat = paFloat32;
			params.device = cur_device->index_in_lib;
			params.hostApiSpecificStreamInfo = nullptr;
			params.suggestedLatency = 0;
			PaError err = Pa_OpenStream(&portaudio_stream, nullptr, &params, dev_sample_rate, chunk_size,
					paNoFlag, &portaudio_stream_request_callback, this);
			_portaudio_test_error(err, "Pa_OpenStream");
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

#if HAS_LIB_PULSEAUDIO
	if (pulse_stream) {
		_pulse_flush_op();
		device_manager->lock();
		pa_stream_set_state_callback(pulse_stream, nullptr, nullptr);
		pa_stream_set_write_callback(pulse_stream, nullptr, nullptr);
		pa_stream_set_underflow_callback(pulse_stream, nullptr, nullptr);

		if (pa_stream_disconnect(pulse_stream) != 0)
			_pulse_test_error("pa_stream_disconnect");

		pa_stream_unref(pulse_stream);
		//_pulse_test_error("pa_stream_unref");
		pulse_stream = nullptr;
		device_manager->unlock();
	}
#endif

#if HAS_LIB_PORTAUDIO
	if (portaudio_stream) {
		device_manager->lock();
		PaError err = Pa_CloseStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_CloseStream");
		portaudio_stream = nullptr;
		device_manager->unlock();
	}
#endif

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

#if HAS_LIB_PULSEAUDIO
// NOT inside lock()/unlock()
void AudioOutput::_pulse_flush_op() {
	if (operation) {
		device_manager->lock();
		pulse_wait_op(session, operation);
		device_manager->unlock();
	}
	operation = nullptr;
}

// NOT inside lock()/unlock()
void AudioOutput::_pulse_start_op(pa_operation *op, const char *msg) {
	if (!op) {
		_pulse_test_error(msg);
		return;
	}
	_pulse_flush_op();
	operation = op;
	//pulse_wait_op(session, op);
}
#endif

void AudioOutput::_pause() {
	if (state != State::PLAYING)
		return;
	session->debug("out", "pause");

#if HAS_LIB_PULSEAUDIO
	if (pulse_stream) {
		device_manager->lock();
		auto op = pa_stream_cork(pulse_stream, true, &pulse_stream_success_callback, this);
		device_manager->unlock();
		_pulse_start_op(op, "pa_stream_cork");
	}
#endif
#if HAS_LIB_PORTAUDIO
	if (portaudio_stream) {
		device_manager->lock();
		//Pa_AbortStream Pa_StopStream
		PaError err = Pa_AbortStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_AbortStream");
		device_manager->unlock();
	}
#endif

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

#if HAS_LIB_PULSEAUDIO
	if (pulse_stream) {
		device_manager->lock();
		pa_operation *op = pa_stream_cork(pulse_stream, false, &pulse_stream_success_callback, this);
		device_manager->unlock();
		_pulse_start_op(op, "pa_stream_cork");
		_pulse_flush_op();
	}
#endif
#if HAS_LIB_PORTAUDIO
	if (portaudio_stream) {
		device_manager->lock();
		PaError err = Pa_StartStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_StartStream");
		device_manager->unlock();
	}
#endif

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
		if (!pulse_stream)
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
		if (!portaudio_stream)
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

#if HAS_LIB_PULSEAUDIO
bool AudioOutput::_pulse_test_error(const char *msg) {
	int e = pa_context_errno(device_manager->pulse_context);
	if (e != 0) {
		session->e(format("AudioOutput: %s: %s", msg, pa_strerror(e)));
		return true;
	}
	return false;
}
#endif

#if HAS_LIB_PORTAUDIO
bool AudioOutput::_portaudio_test_error(PaError err, const char *msg) {
	if (err != paNoError) {
		session->e(format("AudioOutput: %s: %s", msg, Pa_GetErrorText(err)));
		return true;
	}
	return false;
}
#endif

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
			if (pulse_stream) {
				device_manager->lock();
				pa_operation *op = pa_stream_flush(pulse_stream, &pulse_stream_success_callback, this);
				device_manager->unlock();
				_pulse_start_op(op, "pa_stream_flush");
				_pulse_flush_op();

				auto get_write_offset = [this] () {
					if (auto info = pa_stream_get_timing_info(pulse_stream))
						if (info->read_index_corrupt == 0)
							return info->write_index / 8 - samples_offset_since_reset;
					return samples_requested;
				};
				samples_offset_since_reset += get_write_offset();
			}
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
#if HAS_LIB_PULSEAUDIO
	if (device_manager->audio_api == DeviceManager::ApiType::PULSE) {
		// PA_STREAM_INTERPOLATE_TIMING
		if (auto info = pa_stream_get_timing_info(pulse_stream)) {
			auto dbuffer = (info->write_index - info->read_index) / 8;
			if (false)
				printf("%6ld  %6ld  %6ld | w=%-6lld  r=%-6lld  d=%-6ld | req=%-8lld\n", info->configured_sink_usec, info->sink_usec, info->transport_usec, info->write_index/8-samples_offset_since_reset, info->read_index/8-samples_offset_since_reset, dbuffer, samples_requested);
			double samples_per_usec = session->sample_rate() / 1000000.0;
			double delay_samples = (double)(info->sink_usec + info->transport_usec) * samples_per_usec;
			if (info->read_index_corrupt == 0)
				return max(info->write_index / 8 - samples_offset_since_reset - (int64)delay_samples, (int64)0);
			//printf("    %d %d\n", info->write_index, info->read_index);
		}
		/*pa_usec_t t;
		if (pa_stream_get_time(pulse_stream, &t) == 0) {
			double usec2samples = session->sample_rate() / 1000000.0;
			printf("%lld  %.3f\n", fake_samples_played, t/1000000.0);
			return (double)t * usec2samples - fake_samples_played;
		}*/
		return samples_requested;
	}
#endif
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
