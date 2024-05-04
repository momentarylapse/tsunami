/*
 * AudioAccumulator.cpp
 *
 *  Created on: 07.03.2019
 *      Author: michi
 */

#include "AudioAccumulator.h"

#include "../port/Port.h"
#include "../../Session.h"
#include "../../plugins/PluginManager.h"
#include "../../data/base.h"


namespace kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}


void AudioAccumulator::Config::reset() {
	channels = 2;
	accumulate = false;
}

string AudioAccumulator::Config::auto_conf(const string &name) const {
	if (name == "channels")
		return "1:16";
	return "";
}



int AudioAccumulator::Output::read_audio(AudioBuffer& buf) {
	auto source = acc->in.source;
	if (!source)
		return NO_SOURCE;

	int r = source->read_audio(buf);

	if (r <= 0)
		return r;

	if (acc->config.accumulate) {
		// accumulate
		std::lock_guard<std::mutex> lock(acc->mtx_buf);
		//if (buf.channels > acc->buf.channels)
		//	acc->buf.set_channels(buf.channels);
		acc->buf.append(buf.ref(0, r));
	} else {
		// ignore
		acc->samples_skipped += r;
	}
	return r;
}

AudioAccumulator::Output::Output(AudioAccumulator *a) : Port(SignalType::AUDIO, "out") {
	acc = a;
}

AudioAccumulator::AudioAccumulator() :
	Module(ModuleCategory::PLUMBING, "AudioAccumulator")
{
	port_out.add(new Output(this));



	auto _class = session->plugin_manager->get_class("AudioAccumulatorConfig");
	if (_class->elements.num == 0) {
		kaba::add_class(_class);
		kaba::class_add_element("accumulate", kaba::TypeBool, &Config::accumulate);
		kaba::class_add_element("channels", kaba::TypeInt, &Config::channels);
		_class->_vtable_location_target_ = kaba::get_vtable(&config);
	}
	config.kaba_class = _class;
}

ModuleConfiguration *AudioAccumulator::get_config() const {
	return (ModuleConfiguration*)&config;
}

void AudioAccumulator::_accumulate(bool enable) {
	config.accumulate = enable;
	changed();
}

void AudioAccumulator::reset_state() {
	samples_skipped = 0;
}

void AudioAccumulator::set_channels(int _channels) {
	config.channels = _channels;
	changed();
}

void AudioAccumulator::on_config() {
	buf.set_channels(config.channels);
}

base::optional<int64> AudioAccumulator::command(ModuleCommand cmd, int64 param) {
	if (cmd == ModuleCommand::ACCUMULATION_START) {
		_accumulate(true);
		return 0;
	} else if (cmd == ModuleCommand::ACCUMULATION_STOP) {
		_accumulate(false);
		return 0;
	} else if (cmd == ModuleCommand::ACCUMULATION_CLEAR) {
		std::lock_guard<std::mutex> lock(mtx_buf);
		samples_skipped += buf.length;
		buf.clear();
		//buf.set_channels(2);
		return 0;
	} else if (cmd == ModuleCommand::ACCUMULATION_GET_SIZE) {
		std::lock_guard<std::mutex> lock(mtx_buf);
		return buf.length;
	} else if (cmd == ModuleCommand::SET_INPUT_CHANNELS) {
		set_channels(param);
		return 0;
	}
	return base::None;
}
