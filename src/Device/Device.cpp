/*
 * Device.cpp
 *
 *  Created on: 13.04.2016
 *      Author: michi
 */

#include "Device.h"
#include "../lib/hui/hui.h"


Device::Device()
{
	type = -1;
	channels = 0;
	present = false;
	present_old = false;
	visible = true;
	latency = 0;
	client = port = -1;
}

Device::Device(int _type, const string &_name, const string &_internal_name, int _channels)
{
	type = _type;
	name = _name;
	internal_name = _internal_name;
	channels = _channels;
	present = false;
	present_old = false;
	visible = true;
	latency = 0;
	client = port = -1;
}

Device::Device(int _type, const string &s)
{
	Array<string> c = s.explode(",");
	type = _type;
	channels = 0;
	present = false;
	present_old = false;
	visible = true;
	latency = 0;
	client = port = -1;
	if (c.num >= 5){
		name = c[0].replace("${COMMA}", ",").replace("${PIPE}", "|");
		internal_name = c[1].replace("${COMMA}", ",").replace("${PIPE}", "|");
		channels = c[2]._int();
		visible = c[3]._bool();
		latency = c[4]._float();
	}
}

string Device::get_name() const
{
	if (is_default()){
		if (type == Type::AUDIO_OUTPUT)
			return _("        - Default -");
		if (type == Type::AUDIO_INPUT)
			return _("        - Default -");
		if (type == Type::MIDI_INPUT)
			return _("        - don't connect -");
	}
	return name;
}

bool Device::is_default() const
{
	return (internal_name == ":default:");
}

string Device::to_config()
{
	string r;
	r += name.replace(",", "${COMMA}").replace("|", "${PIPE}") + ",";
	r += internal_name.replace(",", "${COMMA}").replace("|", "${PIPE}") + ",";
	r += i2s(channels) + ",";
	r += b2s(visible) + ",";
	r += f2s(latency, 6);
	return r;
}
