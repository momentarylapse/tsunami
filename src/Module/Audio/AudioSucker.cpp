/*
 * AudioSucker.cpp
 *
 *  Created on: 17.09.2017
 *      Author: michi
 */

#include "AudioSucker.h"
#include "../Port/Port.h"
#include "../ModuleFactory.h"
#include "../../Data/base.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../../Session.h"
#include "../../Plugins/PluginManager.h"


namespace kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}



void AudioSucker::Config::reset() {
	channels = 2;
}

string AudioSucker::Config::auto_conf(const string &name) const {
	if (name == "channels")
		return "1:16";
	return "";
}


AudioSucker::AudioSucker(Session *session) :
	Module(ModuleCategory::PLUMBING, "AudioSucker")
{
	port_in.add({SignalType::AUDIO, &source, "in"});
	source = nullptr;


	auto _class = session->plugin_manager->get_class("AudioSuckerConfig");
	if (_class->elements.num == 0) {
		kaba::add_class(_class);
		kaba::class_add_elementx("channels", kaba::TypeInt, &Config::channels);
		_class->_vtable_location_target_ = kaba::get_vtable(&config);
	}
	config.kaba_class = _class;
}

ModuleConfiguration *AudioSucker::get_config() const {
	return (ModuleConfiguration*)&config;
}

int AudioSucker::do_suck(int buffer_size) {
	AudioBuffer temp;
	temp.set_channels(config.channels);
	temp.resize(buffer_size);
	return source->read_audio(temp);
}

void AudioSucker::set_channels(int _channels) {
	config.channels = _channels;
	changed();
}

int AudioSucker::command(ModuleCommand cmd, int param) {
	if (cmd == ModuleCommand::SUCK)
		return do_suck(param);
	if (cmd == ModuleCommand::SET_INPUT_CHANNELS) {
		set_channels(param);
		return 0;
	}
	return COMMAND_NOT_HANDLED;
}

