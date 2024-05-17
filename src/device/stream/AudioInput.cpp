/*
 * AudioInput.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioInput.h"
#include "../pulse/AudioInputStreamPulse.h"
#include "../Device.h"
#include "../DeviceManager.h"
#include "../../lib/hui/hui.h"
#include "../../Session.h"
#include "../../data/base.h"
#include "../../lib/kaba/lib/extern.h"
#include "../../plugins/PluginManager.h"

namespace kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}

#if HAS_LIB_PORTAUDIO
#include <portaudio.h>
#endif


// device
//   config.device sets cur_device
//   config.device auto selects
//   don't set to null!



namespace os {
	extern void require_main_thread(const string&);
}


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
	} else {
		hui::run_later(0.001f, [=] { input->session->w("stream callback error"); });
	}
	return 0;
}

#endif



int AudioInput::read_audio(int port, AudioBuffer &buf) {
	//buf.set_channels(stream->num_channels);
	// reader (AudioSucker) decides on number of channel!

	//printf("read %d %d\n", buf.length, stream->buffer.available());
	if (shared_data.buffer.available() < buf.length)
		return NOT_ENOUGH_DATA;

	return shared_data.buffer.read(buf);
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
		Module(ModuleCategory::STREAM, "AudioInput") {
	session = _session;
	_sample_rate = session->sample_rate();
	num_channels = 2;

	state = State::NO_DEVICE;
#if HAS_LIB_PORTAUDIO
	portaudio_stream = nullptr;
#endif

	dev_man = session->device_manager;
	auto *device_pointer_class = session->plugin_manager->get_class("Device*");
	auto _class = session->plugin_manager->get_class("AudioInputConfig");
	if (_class->elements.num == 0) {
		kaba::add_class(_class);
		kaba::class_add_element("device", device_pointer_class, &Config::device);
		_class->_vtable_location_target_ = kaba::get_vtable(&config);
	}
	config.kaba_class = _class;

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
		shared_data.chunk_size = size;
	else
		shared_data.chunk_size = AudioInputStream::DEFAULT_CHUNK_SIZE;
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

	delete stream;
	stream = nullptr;


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
	os::require_main_thread("in.stop");
	session->i(_("capture audio stop"));
	_pause();
}

void AudioInput::_pause() {
	if (state != State::CAPTURING)
		return;
	session->debug("input", "pause");
	dev_man->lock();

	stream->pause();

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
	if (!session->device_manager->audio_api_initialized())
		return;

	session->debug("input", "create device");

	num_channels = cur_device->channels;
	shared_data.buffer.set_channels(num_channels);
	dev_man->lock();

#if HAS_LIB_PULSEAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PULSE) {
		stream = new AudioInputStreamPulse(session, cur_device, shared_data);
	}
#endif

#if HAS_LIB_PORTAUDIO
	if (dev_man->audio_api == DeviceManager::ApiType::PORTAUDIO) {

		int chunk_size = hui::config.get_int("portaudio.chunk-size", 256);
		// on windows, some devices will stutter, due to some mysterious limit of 100 requests/s
		// nah, that doesn't work either:
		/*paFramesPerBufferUnspecified*/

		if (cur_device->is_default()) {
			session->i(format("open def stream %d  %d", _sample_rate, num_channels));
			PaError err = Pa_OpenDefaultStream(&portaudio_stream, num_channels, 0, paFloat32, _sample_rate, chunk_size,
					&portaudio_stream_request_callback, this);
			_portaudio_test_error(err, "Pa_OpenDefaultStream");
		} else {
			session->i(format("open stream %d  %d", _sample_rate, num_channels));
			PaStreamParameters params;
			params.channelCount = num_channels;
			params.sampleFormat = paFloat32;
			params.device = cur_device->index_in_lib;
			params.hostApiSpecificStreamInfo = nullptr;
			params.suggestedLatency = 0;
			PaError err = Pa_OpenStream(&portaudio_stream, &params, nullptr, _sample_rate, chunk_size,
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

	stream->unpause();

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
	os::require_main_thread("in.start");
	session->i(_("capture audio start"));
	if (state == State::NO_DEVICE)
		_create_dev();

	_unpause();
	return state == State::CAPTURING;
}


#if HAS_LIB_PORTAUDIO
bool AudioInput::_portaudio_test_error(PaError err, const char *msg) {
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

base::optional<int64> AudioInput::command(ModuleCommand cmd, int64 param) {
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
	return base::None;
}

base::optional<int64> AudioInput::samples_recorded() {
	if (state == State::NO_DEVICE)
		return base::None;
	return stream->estimate_samples_captured();
}

ModuleConfiguration *AudioInput::get_config() const {
	return (ModuleConfiguration*)&config;
}
