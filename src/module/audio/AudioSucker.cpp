/*
 * AudioSucker.cpp
 *
 *  Created on: 17.09.2017
 *      Author: michi
 */

#include "AudioSucker.h"
#include "../port/Port.h"
#include "../ModuleFactory.h"
#include "../../data/base.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../Session.h"
#include "../../plugins/PluginManager.h"


namespace kaba {
	VirtualTable* get_vtable(const VirtualBase *p);
}

namespace tsunami {

void AudioSucker::Config::reset() {
	channels = 2;
}

string AudioSucker::Config::auto_conf(const string &name) const {
	if (name == "channels")
		return "1:16";
	return "";
}


AudioSucker::AudioSucker(Session *session) :
	Module(ModuleCategory::Plumbing, "AudioSucker")
{
	auto _class = session->plugin_manager->get_class("AudioSuckerConfig");
	if (_class->elements.num == 0) {
		kaba::add_class(_class);
		kaba::class_add_element("channels", kaba::TypeInt32, &Config::channels);
		_class->_vtable_location_target_ = kaba::get_vtable(&config);
	}
	config.kaba_class = _class;
}

ModuleConfiguration *AudioSucker::get_config() const {
	return (ModuleConfiguration*)&config;
}

int AudioSucker::do_suck(int buffer_size) {
	if (!in.source)
		return Return::NoSource;
	AudioBuffer temp;
	temp.set_channels(config.channels);
	temp.resize(buffer_size);
	return in.source->read_audio(temp);
}

void AudioSucker::set_channels(int _channels) {
	config.channels = _channels;
	changed();
}

base::optional<int64> AudioSucker::command(ModuleCommand cmd, int64 param) {
	if (cmd == ModuleCommand::Suck)
		return do_suck(param);
	if (cmd == ModuleCommand::SetInputChannels) {
		set_channels(param);
		return 0;
	}
	return base::None;
}

}

