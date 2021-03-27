/*
 * AudioInput.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioInput.h"

#include "../../lib/hui/hui.h"
#include "../../Session.h"
#include "../../View/AudioView.h"

#include "../../Stuff/BackupManager.h"
#include "../../Data/base.h"
#include "../Device.h"
#include "../DeviceManager.h"
#include "../../lib/kaba/lib/common.h"
#include "../../Plugins/PluginManager.h"

namespace kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}

#if HAS_LIB_PULSEAUDIO
#include <pulse/pulseaudio.h>
#endif

#if HAS_LIB_PORTAUDIO
#include <portaudio.h>
#endif



static const int DEFAULT_CHUNK_SIZE = 512;

extern void require_main_thread(const string&);

#if HAS_LIB_PULSEAUDIO
extern void pulse_wait_op(Session *session, pa_operation *op); // -> DeviceManager.cpp
extern bool pulse_wait_stream_ready(pa_stream *s, DeviceManager *dm); // -> OutputStream.cpp


void AudioInput::pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata) {
	//printf("input request %d\n", (int)nbytes);
	auto input = static_cast<AudioInput*>(userdata);

	const void *data;
	pa_stream_peek(p, &data, &nbytes);
	input->_pulse_test_error("pa_stream_peek");

	// empty buffer
	if (nbytes == 0)
		return;

	int frames = nbytes / sizeof(float) / input->num_channels;

	if (data) {
		if (input->is_capturing()) {
			float *in = (float*)data;

			RingBuffer &buf = input->buffer;
			AudioBuffer b;
			buf.write_ref(b, frames);
			b.deinterleave(in, input->num_channels);
			buf.write_ref_done(b);

			int done = b.length;
			if (done < frames) {
				buf.write_ref(b, frames - done);
				b.deinterleave(&in[input->num_channels * done], input->num_channels);
				buf.write_ref_done(b);
			}
		}

		pa_stream_drop(p);
		input->_pulse_test_error("pa_stream_drop");
	} else if (nbytes > 0) {
		// holes
		pa_stream_drop(p);
		input->_pulse_test_error("pa_stream_drop");
	}
	//msg_write(">");
	//pa_threaded_mainloop_signal(input->dev_man->pulse_mainloop, 0);
}

void AudioInput::pulse_stream_success_callback(pa_stream *s, int success, void *userdata) {
	auto stream = static_cast<AudioInput*>(userdata);
	//msg_write("--success");
	pa_threaded_mainloop_signal(stream->dev_man->pulse_mainloop, 0);
}

void AudioInput::pulse_stream_state_callback(pa_stream *s, void *userdata) {
	auto stream = static_cast<AudioInput*>(userdata);
	//printf("--state\n");
	pa_threaded_mainloop_signal(stream->dev_man->pulse_mainloop, 0);
}

void AudioInput::pulse_input_notify_callback(pa_stream *p, void *userdata) {
	auto stream = static_cast<AudioInput*>(userdata);
	printf("sstate... %p:  ", p);
	int s = pa_stream_get_state(p);
	if (s == PA_STREAM_UNCONNECTED)
		printf("unconnected");
	if (s == PA_STREAM_READY)
		printf("ready");
	if (s == PA_STREAM_TERMINATED)
		printf("terminated");
	printf("\n");
	pa_threaded_mainloop_signal(stream->dev_man->pulse_mainloop, 0);
}

#endif


#if HAS_LIB_PORTAUDIO

int AudioInput::portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
                                                    unsigned long frames,
                                                    const PaStreamCallbackTimeInfo* timeInfo,
                                                    PaStreamCallbackFlags statusFlags,
                                                    void *userData) {
	//printf("request %d\n", (int)frames);
	auto input = static_cast<AudioInput*>(userData);

	(void)outputBuffer; /* Prevent unused variable warning. */
	auto in = static_cast<const float*>(inputBuffer);


	if (in) {
		if (input->is_capturing()) {

			RingBuffer &buf = input->buffer;
			AudioBuffer b;
			buf.write_ref(b, frames);
			b.deinterleave(in, input->num_channels);
			buf.write_ref_done(b);

			int done = b.length;
			if (done < (int)frames) {
				buf.write_ref(b, frames - done);
				b.deinterleave(&in[input->num_channels * done], input->num_channels);
				buf.write_ref_done(b);
			}
		}
	}
	return 0;
}

#endif



AudioInput::Output::Output(AudioInput *s) :
	Port(SignalType::AUDIO, "out") {
	stream = s;
}

int AudioInput::Output::read_audio(AudioBuffer &buf) {
	buf.set_channels(stream->num_channels);

	//printf("read %d %d\n", buf.length, stream->buffer.available());
	if (stream->buffer.available() < buf.length)
		return NOT_ENOUGH_DATA;

	int r = stream->buffer.read(buf);

	return r;
}

void AudioInput::Config::reset() {
	device = _module->session->device_manager->choose_device(DeviceType::AUDIO_INPUT);
}

string AudioInput::Config::auto_conf(const string &name) const {
	if (name == "device")
		return "audio:input";
	return "";
}

AudioInput::AudioInput(Session *_session) :
		Module(ModuleCategory::STREAM, "AudioInput"),
		buffer(1048576) {
	session = _session;
	_sample_rate = session->sample_rate();
	chunk_size = DEFAULT_CHUNK_SIZE;
	num_channels = 2;

	state = State::NO_DEVICE;
#if HAS_LIB_PULSEAUDIO
	pulse_stream = nullptr;
#endif
#if HAS_LIB_PORTAUDIO
	portaudio_stream = nullptr;
#endif

	out = new Output(this);
	port_out.add(out);


	dev_man = session->device_manager;
	auto *device_pointer_class = session->plugin_manager->get_class("Device*");
	auto _class = session->plugin_manager->get_class("AudioInputConfig");
	if (_class->elements.num == 0) {
		kaba::add_class(_class);
		kaba::class_add_elementx("device", device_pointer_class, &Config::device);
		_class->_vtable_location_target_ = kaba::get_vtable(&config);
	}
	config._class = _class;

	cur_device = nullptr;
}

AudioInput::~AudioInput() {
	if (state == State::CAPTURING)
		_pause();
	if (state == State::PAUSED)
		_kill_dev();
}

void AudioInput::__init__(Session *session) {
	new(this) AudioInput(session);
}

void AudioInput::__delete__() {
	this->AudioInput::~AudioInput();
}

void AudioInput::set_chunk_size(int size) {
	if (size > 0)
		chunk_size = size;
	else
		chunk_size = DEFAULT_CHUNK_SIZE;
}

Device *AudioInput::get_device() {
	return cur_device;
}

void AudioInput::set_device(Device *device) {
	config.device = device;
	changed();
}

void AudioInput::update_device() {
	auto old_state = state;
	if (state == State::CAPTURING)
		_pause();
	if (state == State::PAUSED)
		_kill_dev();

	cur_device = config.device;
	num_channels = config.device->channels;

	if (old_state == State::CAPTURING)
		start();

	//tsunami->log->info(format("input device '%s' chosen", Pa_GetDeviceInfo(pa_device_no)->name));
}

void AudioInput::on_config() {
	if (cur_device != config.device)
		update_device();
}

void AudioInput::_kill_dev() {
	if (state != State::PAUSED)
		return;
	session->debug("input", "kill device");
	dev_man->lock();

#if HAS_LIB_PULSEAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PULSE) {

		pa_stream_set_state_callback(pulse_stream, nullptr, nullptr);
		pa_stream_set_read_callback(pulse_stream, nullptr, nullptr);

		pa_stream_disconnect(pulse_stream);
		_pulse_test_error("pa_stream_disconnect");

/*		// FIXME really necessary?!?!?!?
		for (int i=0; i<1000; i++) {
			if (pa_stream_get_state(pulse_stream) == PA_STREAM_TERMINATED) {
				break;
			}
			hui::Sleep(0.001f);
		}*/

		pa_stream_unref(pulse_stream);
		_pulse_test_error("pa_stream_unref");
	}
#endif

#if HAS_LIB_PORTAUDIO
	if (portaudio_stream) {
		PaError err = Pa_CloseStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_CloseStream");
		portaudio_stream = nullptr;
	}
#endif

	dev_man->unlock();
	state = State::NO_DEVICE;
}

void AudioInput::stop() {
	require_main_thread("in.stop");
	session->i(_("capture audio stop"));
	_pause();
}

void AudioInput::_pause() {
	if (state != State::CAPTURING)
		return;
	session->debug("input", "pause");
	dev_man->lock();


#if HAS_LIB_PULSEAUDIO
	if (pulse_stream) {
		pa_operation *op = pa_stream_cork(pulse_stream, true, &pulse_stream_success_callback, this);
		_pulse_test_error("pa_stream_cork");
		pulse_wait_op(session, op);
	}
#endif
#if HAS_LIB_PORTAUDIO
	if (portaudio_stream) {
		// often crashes here... might be a bug in the libraries...?!?
		PaError err = Pa_StopStream(portaudio_stream);
		_portaudio_test_error(err, "Pa_StopStream");
	}
#endif

	dev_man->unlock();
	state = State::PAUSED;
}

void AudioInput::_create_dev() {
	if (state != State::NO_DEVICE)
		return;

	session->debug("input", "create device");

	num_channels = cur_device->channels;
	buffer.set_channels(num_channels);
	dev_man->lock();

#if HAS_LIB_PULSEAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PULSE) {
		pa_sample_spec ss;
		ss.rate = _sample_rate;
		ss.channels = num_channels;
		ss.format = PA_SAMPLE_FLOAT32LE;
		pulse_stream = pa_stream_new(session->device_manager->pulse_context, "stream-in", &ss, nullptr);
		_pulse_test_error("pa_stream_new");


		pa_stream_set_read_callback(pulse_stream, &pulse_stream_request_callback, this);
		pa_stream_set_state_callback(pulse_stream, &pulse_stream_state_callback, this);

		pa_buffer_attr attr_in;
	//	attr_in.fragsize = -1;
		attr_in.fragsize = chunk_size;
		attr_in.maxlength = -1;
		attr_in.minreq = -1;
		attr_in.tlength = -1;
		attr_in.prebuf = -1;
		const char *dev = nullptr;
		if (!cur_device->is_default())
			dev = cur_device->internal_name.c_str();
		auto flags = (pa_stream_flags_t)(PA_STREAM_ADJUST_LATENCY|PA_STREAM_AUTO_TIMING_UPDATE|PA_STREAM_INTERPOLATE_TIMING);
		pa_stream_connect_record(pulse_stream, dev, &attr_in, flags);
		// without PA_STREAM_ADJUST_LATENCY, we will get big chunks (split into many small ones, but still "clustered")
		_pulse_test_error("pa_stream_connect_record");

		if (!pulse_wait_stream_ready(pulse_stream, dev_man)) {
			dev_man->unlock();
			session->e("pulse_wait_stream_ready");
			return;
		}
	}
#endif

#if HAS_LIB_PORTAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PORTAUDIO) {
		session->i("open def stream");

		if (cur_device->is_default()) {
			PaError err = Pa_OpenDefaultStream(&portaudio_stream, 2, 0, paFloat32, _sample_rate, 256,
					&portaudio_stream_request_callback, this);
			_portaudio_test_error(err, "Pa_OpenDefaultStream");
		} else {
			PaStreamParameters params;
			params.channelCount = num_channels;
			params.sampleFormat = paFloat32;
			params.device = cur_device->index_in_lib;
			params.hostApiSpecificStreamInfo = nullptr;
			params.suggestedLatency = 0;
			PaError err = Pa_OpenStream(&portaudio_stream, &params, nullptr, _sample_rate, 0,
					paNoFlag, &portaudio_stream_request_callback, this);
			_portaudio_test_error(err, "Pa_OpenStream");
		}
	}
#endif

	dev_man->unlock();
	state = State::PAUSED;
}

void AudioInput::_unpause() {
	if (state != State::PAUSED)
		return;
	session->debug("input", "unpause");
	dev_man->lock();

#if HAS_LIB_PULSEAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PULSE) {
		if (pulse_stream) {
			pa_operation *op = pa_stream_cork(pulse_stream, false, &pulse_stream_success_callback, this);
			_pulse_test_error("pa_stream_cork");
			pulse_wait_op(session, op);
		}
	}
#endif
#if HAS_LIB_PORTAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PORTAUDIO) {
		if (portaudio_stream) {
			PaError err = Pa_StartStream(portaudio_stream);
			_portaudio_test_error(err, "Pa_StartStream");
		}
	}
#endif

	dev_man->unlock();
	state = State::CAPTURING;
}

bool AudioInput::start() {
	require_main_thread("in.start");
	session->i(_("capture audio start"));
	if (state == State::NO_DEVICE)
		_create_dev();

	_unpause();
	return state == State::CAPTURING;
}

bool AudioInput::_pulse_test_error(const string &msg) {
#if HAS_LIB_PULSEAUDIO
	int e = pa_context_errno(session->device_manager->pulse_context);
	if (e != 0)
		session->e(format("%s (input): %s", msg, pa_strerror(e)));
	return (e != 0);
#endif
	return false;
}

#if HAS_LIB_PORTAUDIO
bool AudioInput::_portaudio_test_error(PaError err, const string &msg) {
	if (err != paNoError) {
		session->e(format("%s: (input): %s", msg, Pa_GetErrorText(err)));
		return true;
	}
	return false;
}
#endif


bool AudioInput::is_capturing() {
	return state == State::CAPTURING;
}

int AudioInput::command(ModuleCommand cmd, int param) {
	if (cmd == ModuleCommand::START) {
		start();
		return 0;
	} else if (cmd == ModuleCommand::STOP) {
		stop();
		return 0;
	} else if (cmd == ModuleCommand::PREPARE_START) {
		if (state == State::NO_DEVICE)
			_create_dev();
		return 0;
	}
	return COMMAND_NOT_HANDLED;
}

int64 AudioInput::samples_recorded() {
	if (state == State::NO_DEVICE)
		return 0;
#if HAS_LIB_PULSEAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PULSE) {
		pa_usec_t t;
		pa_stream_get_time(pulse_stream, &t);
		double usec2samples = session->sample_rate() / 1000000.0;
		return (double)t * usec2samples;
	}
#endif
	return 0;
}

ModuleConfiguration *AudioInput::get_config() const {
	return (ModuleConfiguration*)&config;
}
