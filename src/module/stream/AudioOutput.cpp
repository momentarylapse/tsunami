/*
 * AudioOutput.cpp
 *
 *  Created on: 01.11.2014
 *      Author: michi
 */

#include "AudioOutput.h"
#include "../../device/interface/DeviceContext.h"
#include "../../device/interface/AudioOutputStream.h"
#include "../../device/Device.h"
#include "../../device/DeviceManager.h"
#include "../../Session.h"
#include "../../data/base.h"
#include "../../lib/kaba/lib/extern.h"
#include "../../plugins/PluginManager.h"
#include "../../lib/os/time.h"
#include "../../lib/hui/hui.h"

namespace kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}


namespace os {
	extern void require_main_thread(const string &msg);
}

namespace tsunami {

void AudioOutput::Config::reset() {
	device = _module->session->device_manager->choose_device(DeviceType::AudioOutput);
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
	Module(ModuleCategory::Stream, "AudioOutput") {
	state = State::UnpreparedNoDeviceNoData;
	session = _session;

	config.volume = 1;

	shared_data.callback_played_end_of_stream = [this]{ on_played_end_of_stream(); }; // TODO prevent abort before playback really finished

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

	cur_device = nullptr;

	latency = 0;
}

AudioOutput::~AudioOutput() {
	_pause();
	_kill_dev();
}

void AudioOutput::_create_dev() {
	if (has_device())
		return;
	if (!device_manager->audio_api_initialized())
		return;
	session->debug("out", "create device");
	
	stream = device_manager->audio_context->create_audio_output_stream(session, cur_device, &shared_data);

	if (!stream) {
		stop();
		return;
	}

	if (stream->error) {
		delete stream;
		stream = nullptr;
		stop();
		return;
	}

	if (state == State::UnpreparedNoDeviceNoData)
		_set_state(State::UnpreparedNoData);
	else if (state == State::UnpreparedNoDevice)
		_set_state(State::Paused);
}

void AudioOutput::_kill_dev() {
	if (!has_device())
		return;
	session->debug("out", "kill device");

	if (stream) {
		delete stream;
		stream = nullptr;
	}

	shared_data.clear_data_state();
	_set_state(State::UnpreparedNoDeviceNoData);
}


void AudioOutput::stop() {
	os::require_main_thread("out.stop");
	_pause();
}

void AudioOutput::_pause() {
	if (state != State::Playing)
		return;
	session->debug("out", "pause");

	if (stream)
		stream->pause();

	_set_state(State::Paused);
}

void AudioOutput::_set_state(State s) {
	state = s;
	out_state_changed.notify();
}

void AudioOutput::_unpause() {
	if (state != State::Paused)
		return;
	session->debug("out", "unpause");

	shared_data.read_end_of_stream = false;
	shared_data.played_end_of_stream = false;

	if (stream)
		stream->unpause();

	_set_state(State::Playing);
}

int AudioOutput::_read_stream_into_ring_buffer(int buffer_size) {
	if (!in.source)
		return 0;

	int size = 0;
	AudioBuffer b;
	shared_data.ring_buf.write_ref(b, buffer_size);

	// read data
	size = in.source->read_audio(b);

	if (size == Return::NotEnoughData) {
		//printf(" -> no data\n");
		// keep trying...
		shared_data.ring_buf.write_ref_cancel(b);
		return size;
	}

	// out of data?
	if (size == Return::EndOfStream) {
		//printf(" -> end  STREAM\n");
		shared_data.read_end_of_stream = true;
		hui::run_in_gui_thread([this] { on_read_end_of_stream(); });
		shared_data.ring_buf.write_ref_cancel(b);
		return size;
	}

	// add to queue
	b.length = size;
	shared_data.ring_buf.write_ref_done(b);
	shared_data.buffer_is_cleared = false;
//	printf(" -> %d of %d\n", size, buffer_size);
	return size;
}

void AudioOutput::set_device(Device *d) {
	if (!d)
		return;
	config.device = d;
	on_config();
	out_changed.notify();
}

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
	_read_stream_into_ring_buffer(shared_data.prebuffer_size);

	if (!stream)
		return;
	stream->pre_buffer();

	if (state == State::UnpreparedNoDeviceNoData)
		_set_state(State::UnpreparedNoDevice);
	else if (state == State::UnpreparedNoData)
		_set_state(State::Paused);
}

int AudioOutput::get_available() {
	return shared_data.ring_buf.available();
}

float AudioOutput::get_volume() const {
	return config.volume;
}

void AudioOutput::set_volume(float _volume) {
	config.volume = _volume;
	on_config();
	out_state_changed.notify();
	out_changed.notify();
}

void AudioOutput::set_prebuffer_size(int size) {
	shared_data.prebuffer_size = size;
}



void AudioOutput::update_device() {
	auto old_state = state;
	if (state == State::Playing)
		_pause();
	if (has_device())
		_kill_dev();

	cur_device = config.device;

	if (old_state == State::Playing)
		start();
}

void AudioOutput::on_config() {
	if (cur_device != config.device)
		update_device();
	shared_data.volume = config.volume * device_manager->output_volume;
}

void AudioOutput::on_played_end_of_stream() {
	//printf("---------ON PLAY END OF STREAM\n");
	out_play_end_of_stream();
}

void AudioOutput::on_read_end_of_stream() {
	//printf("---------ON READ END OF STREAM\n");
	out_read_end_of_stream();
}

void AudioOutput::reset_state() {
	if (state == State::UnpreparedNoDeviceNoData)
		return;
	if (state == State::Playing)
		_pause();
	os::require_main_thread("out.reset");

	if (state == State::Paused) {
		session->debug("out", "flush");
		if (stream)
			stream->flush();
		_set_state(State::UnpreparedNoData);
	}

	shared_data.clear_data_state();
}

base::optional<int64> AudioOutput::command(ModuleCommand cmd, int64 param) {
	if (cmd == ModuleCommand::Start) {
		start();
		return 0;
	} else if (cmd == ModuleCommand::Stop) {
		stop();
		return 0;
	} else if (cmd == ModuleCommand::PrepareStart) {
		if (!has_data())
			_fill_prebuffer();
		if (!has_device())
			_create_dev();
		return 0;
	} else if (cmd == ModuleCommand::Suck) {
		if (shared_data.ring_buf.available() >= shared_data.prebuffer_size)
			return 0;
		//printf("suck %d    av %d\n", param, ring_buf.available());
		return (int64)_read_stream_into_ring_buffer((int)param);
	} else if (cmd == ModuleCommand::SampleCountMode) {
		return (int64)SampleCountMode::Consumer;
	} else if (cmd == ModuleCommand::GetSampleCount) {
		if (auto s = estimate_samples_played())
			return *s;
		return 0;
	}
	return base::None;
}

bool AudioOutput::is_playing() {
	return (state == State::Playing) or (state == State::Paused);
}

bool AudioOutput::has_data() const {
	return state != State::UnpreparedNoData and state != State::UnpreparedNoDeviceNoData;
}

bool AudioOutput::has_device() const {
	return state != State::UnpreparedNoDevice and state != State::UnpreparedNoDeviceNoData;
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
		if (auto r = stream->estimate_samples_played())
			return *r;
	return shared_data.samples_requested;
	//return base::None;
}

// requested by audio library (since last reset_state())
int64 AudioOutput::get_samples_requested() const {
	return shared_data.samples_requested;
}

ModuleConfiguration *AudioOutput::get_config() const {
	return (ModuleConfiguration*)&config;
}

}
