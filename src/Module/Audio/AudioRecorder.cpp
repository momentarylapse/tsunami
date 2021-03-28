/*
 * AudioRecorder.cpp
 *
 *  Created on: 07.03.2019
 *      Author: michi
 */

#include "AudioRecorder.h"
#include "../Port/Port.h"
#include "../../Session.h"
#include "../../Plugins/PluginManager.h"
#include "../../Data/base.h"


namespace kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}


void AudioRecorder::Config::reset() {
	channels = 2;
	accumulating = false;
}

string AudioRecorder::Config::auto_conf(const string &name) const {
	if (name == "channels")
		return "1:16";
	return "";
}



int AudioRecorder::Output::read_audio(AudioBuffer& buf) {
	if (!rec->source)
		return buf.length;

	int r = rec->source->read_audio(buf);

	if (r <= 0)
		return r;

	if (rec->config.accumulating) {
		// accumulate
		std::lock_guard<std::mutex> lock(rec->mtx_buf);
		if (buf.channels > rec->buf.channels)
			rec->buf.set_channels(buf.channels);
		rec->buf.append(buf.ref(0, r));
	} else {
		// ignore
		rec->samples_skipped += r;
	}
	return r;
}

AudioRecorder::Output::Output(AudioRecorder *r) : Port(SignalType::AUDIO, "out") {
	rec = r;
}

AudioRecorder::AudioRecorder() :
	Module(ModuleCategory::PLUMBING, "AudioRecorder")
{
	port_out.add(new Output(this));
	port_in.add({SignalType::AUDIO, &source, "in"});
	source = nullptr;



	auto _class = session->plugin_manager->get_class("AudioRecorderConfig");
	if (_class->elements.num == 0) {
		kaba::add_class(_class);
		kaba::class_add_elementx("accumulating", kaba::TypeBool, &Config::accumulating);
		kaba::class_add_elementx("channels", kaba::TypeInt, &Config::channels);
		_class->_vtable_location_target_ = kaba::get_vtable(&config);
	}
	config.kaba_class = _class;
}

ModuleConfiguration *AudioRecorder::get_config() const {
	return (ModuleConfiguration*)&config;
}

void AudioRecorder::_accumulate(bool enable) {
	config.accumulating = enable;
	notify();
}

void AudioRecorder::reset_state() {
	samples_skipped = 0;
}

void AudioRecorder::set_channels(int _channels) {
	config.channels = _channels;
	notify();
}

void AudioRecorder::on_config() {
	buf.set_channels(config.channels);
}

int AudioRecorder::command(ModuleCommand cmd, int param) {
	if (cmd == ModuleCommand::ACCUMULATION_START) {
		_accumulate(true);
		return 0;
	} else if (cmd == ModuleCommand::ACCUMULATION_STOP) {
		_accumulate(false);
		return 0;
	} else if (cmd == ModuleCommand::ACCUMULATION_CLEAR) {
		samples_skipped += buf.length;
		buf.clear();
		//buf.set_channels(2);
		return 0;
	} else if (cmd == ModuleCommand::ACCUMULATION_GET_SIZE) {
		return buf.length;
	} else if (cmd == ModuleCommand::SET_INPUT_CHANNELS) {
		set_channels(param);
		return 0;
	}
	return COMMAND_NOT_HANDLED;
}
