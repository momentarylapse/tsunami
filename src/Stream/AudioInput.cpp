/*
 * AudioInput.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioInput.h"

#include "../lib/hui/hui.h"
#include "../Session.h"
#include "../View/AudioView.h"

#include "../Stuff/BackupManager.h"
#include "../Data/base.h"
#include "../Device/Device.h"
#include "../Device/DeviceManager.h"
#include "../lib/kaba/lib/common.h"
#include "../Plugins/PluginManager.h"

namespace Kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}

#if HAS_LIB_PULSEAUDIO
#include <pulse/pulseaudio.h>
#endif

#if HAS_LIB_PORTAUDIO
#include <portaudio.h>
#endif


float AudioInput::playback_delay_const;


static const int DEFAULT_CHUNK_SIZE = 512;

extern void require_main_thread(const string&);

#if HAS_LIB_PULSEAUDIO
extern void pulse_wait_op(Session *session, pa_operation *op); // -> DeviceManager.cpp
extern bool pulse_wait_stream_ready(pa_stream *s); // -> OutputStream.cpp


void AudioInput::pulse_stream_request_callback(pa_stream *p, size_t nbytes, void *userdata) {
	//printf("input request %d\n", (int)nbytes);
	AudioInput *input = (AudioInput*)userdata;

	const void *data;
	pa_stream_peek(p, &data, &nbytes);
	input->_pulse_test_error("pa_stream_peek");
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
	}
	//msg_write(">");
}
void input_notify_callback(pa_stream *p, void *userdata) {
	printf("sstate... %p:  ", p);
	int s = pa_stream_get_state(p);
	if (s == PA_STREAM_UNCONNECTED)
		printf("unconnected");
	if (s == PA_STREAM_READY)
		printf("ready");
	if (s == PA_STREAM_TERMINATED)
		printf("terminated");
	printf("\n");
}


void input_success_callback(pa_stream *s, int success, void *userdata) {
	msg_write("--success");
}
#endif


#if HAS_LIB_PORTAUDIO

int AudioInput::portaudio_stream_request_callback(const void *inputBuffer, void *outputBuffer,
                                                    unsigned long frames,
                                                    const PaStreamCallbackTimeInfo* timeInfo,
                                                    PaStreamCallbackFlags statusFlags,
                                                    void *userData) {
	//printf("request %d\n", (int)frames);
	AudioInput *input = (AudioInput*)userData;

	(void)outputBuffer; /* Prevent unused variable warning. */
	float *in = (float*) inputBuffer;


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



void AudioInput::SyncData::reset() {
	num_points = 0;
	delay_sum = 0;
	samples_in = 0;
	/*if (tsunami->output->IsPlaying())
		offset_out = tsunami->output->GetRange().offset;*/ // TODO
}

void AudioInput::SyncData::add(int samples) {
#if 0
	if (tsunami->win->view->isPlaying()) {
		samples_in += samples;
		/*delay_sum += (tsunami->output->GetPos() - offset_out - samples_in);*/ // TODO
		num_points ++;
	}
#endif
}

int AudioInput::SyncData::get_delay() {
	if (num_points > 0)
		return (delay_sum / num_points);
	return 0;
}

AudioInput::Output::Output(AudioInput *s) :
	Port(SignalType::AUDIO, "out") {
	stream = s;
}

int AudioInput::Output::read_audio(AudioBuffer &buf) {
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
		Module(ModuleType::STREAM, "AudioInput"),
		buffer(1048576) {
	session = _session;
	_sample_rate = session->sample_rate();
	chunk_size = DEFAULT_CHUNK_SIZE;
	num_channels = 0;

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
		Kaba::add_class(_class);
		Kaba::class_add_elementx("device", device_pointer_class, &Config::device);
		_class->_vtable_location_target_ = Kaba::get_vtable(&config);
	}
	config._class = _class;

	playback_delay_const = 0;
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
	playback_delay_const = cur_device->latency;
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

#if HAS_LIB_PULSEAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PULSE) {
		pa_stream_disconnect(pulse_stream);
		_pulse_test_error("pa_stream_disconnect");

		for (int i=0; i<1000; i++) {
			if (pa_stream_get_state(pulse_stream) == PA_STREAM_TERMINATED) {
				break;
			}
			hui::Sleep(0.001f);
		}

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


#if HAS_LIB_PULSEAUDIO
	if (pulse_stream) {
		pa_operation *op = pa_stream_cork(pulse_stream, true, nullptr, nullptr);
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

	state = State::PAUSED;
	//buffer.clear();
}

void AudioInput::_create_dev() {
	if (state != State::NO_DEVICE)
		return;

	session->debug("input", "create device");

	num_channels = min(cur_device->channels, 2);

#if HAS_LIB_PULSEAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PULSE) {
		pa_sample_spec ss;
		ss.rate = _sample_rate;
		ss.channels = num_channels;
		ss.format = PA_SAMPLE_FLOAT32LE;
		pulse_stream = pa_stream_new(session->device_manager->pulse_context, "stream-in", &ss, nullptr);
		_pulse_test_error("pa_stream_new");


		pa_stream_set_read_callback(pulse_stream, &pulse_stream_request_callback, this);
		//pa_stream_set_state_callback(pulse_stream, &input_notify_callback, NULL);

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
		pa_stream_connect_record(pulse_stream, dev, &attr_in, (pa_stream_flags_t)PA_STREAM_ADJUST_LATENCY);
		// without PA_STREAM_ADJUST_LATENCY, we will get big chunks (split into many small ones, but still "clustered")
		_pulse_test_error("pa_stream_connect_record");

		if (!pulse_wait_stream_ready(pulse_stream)) {
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

	state = State::PAUSED;
}

void AudioInput::_unpause() {
	if (state != State::PAUSED)
		return;
	session->debug("input", "unpause");

#if HAS_LIB_PULSEAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PULSE) {
		if (pulse_stream) {
			pa_operation *op = pa_stream_cork(pulse_stream, false, nullptr, nullptr);
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
		session->e(msg + " (input): " + pa_strerror(e));
	return (e != 0);
#endif
	return false;
}

#if HAS_LIB_PORTAUDIO
bool AudioInput::_portaudio_test_error(PaError err, const string &msg) {
	if (err != paNoError) {
		session->e(msg + ": (input): " + Pa_GetErrorText(err));
		return true;
	}
	return false;
}
#endif

float AudioInput::get_playback_delay_const() {
	return playback_delay_const;
}

void AudioInput::set_playback_delay_const(float f) {
	playback_delay_const = f;
	hui::Config.set_float("Input.PlaybackDelay", playback_delay_const);
}

void AudioInput::reset_sync() {
	sync.reset();
}

bool AudioInput::is_capturing() {
	return state == State::CAPTURING;
}

int AudioInput::get_delay() {
	return sync.get_delay() - playback_delay_const * (float)_sample_rate / 1000.0f;
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

ModuleConfiguration *AudioInput::get_config() const {
	return (ModuleConfiguration*)&config;
}
