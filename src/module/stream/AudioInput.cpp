/*
 * AudioInput.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioInput.h"
#include "../../device/interface/DeviceContext.h"
#include "../../device/interface/AudioInputStream.h"
#include "../../device/Device.h"
#include "../../device/DeviceManager.h"
#include "../../Session.h"
#include "../../data/base.h"
#include "../../lib/kaba/lib/extern.h"
#include "../../plugins/PluginManager.h"

namespace kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}


// device
//   config.device sets cur_device
//   config.device auto selects
//   don't set to null!



namespace os {
	extern void require_main_thread(const string&);
}

namespace tsunami {


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
	shared_data.num_channels = 2;

	state = State::NO_DEVICE;

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
	shared_data.num_channels = config.device->channels;

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

	delete stream;
	stream = nullptr;

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

	stream->pause();

	state = State::PAUSED;
}

void AudioInput::_create_dev() {
	if (state != State::NO_DEVICE)
		return;
	if (!session->device_manager->audio_api_initialized())
		return;

	session->debug("input", "create device");

	shared_data.num_channels = cur_device->channels;
	shared_data.buffer.set_channels(shared_data.num_channels);

	stream = dev_man->audio_context->create_audio_input_stream(session, cur_device, &shared_data);

	state = State::PAUSED;
}

void AudioInput::_unpause() {
	if (state != State::PAUSED)
		return;
	session->debug("input", "unpause");

	stream->unpause();

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

}
