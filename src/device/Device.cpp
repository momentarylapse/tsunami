/*
 * Device.cpp
 *
 *  Created on: 13.04.2016
 *      Author: michi
 */

#include "Device.h"

#include "../lib/any/any.h"
#include "../lib/hui/language.h"

namespace tsunami {

Device::Device() {
	type = (DeviceType)-1;
	channels = 0;
	present = false;
	present_old = false;
	visible = true;
	client = port = -1;
	index_in_lib = -1;
	default_by_lib = false;
}

Device::Device(DeviceType _type, const string &_name, const string &_internal_name, int _channels) {
	type = _type;
	name = _name;
	internal_name = _internal_name;
	channels = _channels;
	present = false;
	present_old = false;
	visible = true;
	client = port = -1;
	index_in_lib = -1;
	default_by_lib = false;
}

Device::Device(DeviceType _type, const Any &a) {
	type = _type;
	channels = 0;
	present = false;
	present_old = false;
	visible = true;
	client = port = -1;
	index_in_lib = -1;
	default_by_lib = false;

	if (a.has("name"))
		name = str(a["name"]);
	if (a.has("internal"))
		internal_name = str(a["internal"]);
	if (a.has("channels"))
		channels = a["channels"]._int();
	if (a.has("visible"))
		visible = a["visible"]._bool();
}

string Device::get_name() const {
	if (is_default()) {
		/*if (type == DeviceType::AudioOutput)
			return _("        - Default -");
		if (type == DeviceType::AudioInput)
			return _("        - Default -");*/
		if (type == DeviceType::MidiInput)
			return _("        - don't connect -");
		return name + " (default)";
	}
	return name;
}

bool Device::is_default() const {
	return default_by_lib;
}

Any Device::to_config() const {
	Any a = Any::EmptyDict;
	a["name"] = name;
	a["internal"] = internal_name;
	a["channels"] = channels;
	a["visible"] = visible;
	return a;
}

}
