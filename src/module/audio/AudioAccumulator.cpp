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

namespace tsunami {


void AudioAccumulator::Config::reset() {
	channels = 2;
	accumulate = false;
}

string AudioAccumulator::Config::auto_conf(const string &name) const {
	if (name == "channels")
		return "1:16";
	return "";
}



int AudioAccumulator::read_audio(int port, AudioBuffer& buf) {
	auto source = in.source;
	if (!source)
		return Return::NoSource;

	int r = source->read_audio(buf);

	if (r <= 0)
		return r;

	if (config.accumulate) {
		// accumulate
		std::lock_guard<std::mutex> lock(mtx_buf);
		//if (buf.channels > acc->buf.channels)
		//	buf.set_channels(buf.channels);
		buffer.append(buf.ref(0, r));
	} else {
		// ignore
		samples_skipped += r;
	}
	return r;
}

AudioAccumulator::AudioAccumulator() :
	Module(ModuleCategory::Plumbing, "AudioAccumulator")
{
	auto _class = session->plugin_manager->get_class("AudioAccumulatorConfig");
	if (_class->elements.num == 0) {
		kaba::add_class(_class);
		kaba::class_add_element("accumulate", kaba::TypeBool, &Config::accumulate);
		kaba::class_add_element("channels", kaba::TypeInt32, &Config::channels);
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
	buffer.set_channels(config.channels);
}

base::optional<int64> AudioAccumulator::command(ModuleCommand cmd, int64 param) {
	if (cmd == ModuleCommand::AccumulationStart) {
		_accumulate(true);
		return 0;
	} else if (cmd == ModuleCommand::AccumulationStop) {
		_accumulate(false);
		return 0;
	} else if (cmd == ModuleCommand::AccumulationClear) {
		std::lock_guard<std::mutex> lock(mtx_buf);
		samples_skipped += buffer.length;
		buffer.clear();
		//buf.set_channels(2);
		return 0;
	} else if (cmd == ModuleCommand::AccumulationGetSize) {
		std::lock_guard<std::mutex> lock(mtx_buf);
		return buffer.length;
	} else if (cmd == ModuleCommand::SetInputChannels) {
		set_channels((int)param);
		return 0;
	}
	return base::None;
}

}
