/*
 * AudioChannelSelector.cpp
 *
 *  Created on: Mar 28, 2021
 *      Author: michi
 */

#include "AudioChannelSelector.h"
#include "../../Data/base.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../Stuff/BackupManager.h"
#include "../../lib/math/math.h"
#include "../../Session.h"
#include "../../Plugins/PluginManager.h"


namespace kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}



void AudioChannelSelector::Config::reset() {
	channels = 2;
	map = {0,1};
}

string AudioChannelSelector::Config::auto_conf(const string &name) const {
	if (name == "channels")
		return "1:16";
	return "";
}



AudioChannelSelector::AudioChannelSelector() : Module(ModuleCategory::PLUMBING, "AudioChannelSelector") {
	out = new Output(this);
	port_out.add(out);
	port_in.add({SignalType::AUDIO, &source, "in"});
	source = nullptr;

	peak_meter = new PeakMeter;

	auto _class = session->plugin_manager->get_class("AudioChannelSelectorConfig");
	if (_class->elements.num == 0) {
		kaba::add_class(_class);
		kaba::class_add_elementx("channels", kaba::TypeInt, &Config::channels);
		_class->_vtable_location_target_ = kaba::get_vtable(&config);
	}
	config.kaba_class = _class;
}

ModuleConfiguration *AudioChannelSelector::get_config() const {
	return (ModuleConfiguration*)&config;
}

int AudioChannelSelector::Output::read_audio(AudioBuffer& buf) {
	if (!cs->source)
		return buf.length;

	AudioBuffer buf_in;
	buf_in.set_channels(cs->config.channels);
	buf_in.resize(buf.length);
	int r = cs->source->read_audio(buf_in);

	if (r > 0) {
		if (cs->peak_meter)
			cs->peak_meter->process(buf_in);
		cs->apply(buf_in.ref(0, r), buf);
	}

	return r;
}

AudioChannelSelector::Output::Output(AudioChannelSelector *_cs) : Port(SignalType::AUDIO, "out") {
	cs = _cs;
}

void AudioChannelSelector::set_channel_map(int _n_in, const Array<int> &_map) {
	config.channels = _n_in;
	config.map = _map;
	changed();
}

void AudioChannelSelector::apply(const AudioBuffer &buf_in, AudioBuffer &buf_out) {
	for (int o=0; o<buf_out.channels; o++) {
		int i = min(o, buf_in.channels - 1);
		if (o < config.map.num)
			i = clamp(config.map[o], 0, buf_in.channels - 1);
		memcpy(&buf_out.c[o][0], &buf_in.c[i][0], sizeof(float) * buf_in.length);
	}
}

int AudioChannelSelector::command(ModuleCommand cmd, int param) {
	if (cmd == ModuleCommand::SET_INPUT_CHANNELS) {
		config.channels = param;
		peak_meter->command(cmd, param);
		changed();
		return 0;
	}
	return COMMAND_NOT_HANDLED;
}


