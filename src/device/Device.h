/*
 * Device.h
 *
 *  Created on: 13.04.2016
 *      Author: michi
 */

#ifndef SRC_DEVICE_DEVICE_H_
#define SRC_DEVICE_DEVICE_H_

#include "../lib/base/base.h"

class Any;

namespace tsunami {

enum class DeviceType {
	AudioOutput,
	AudioInput,
	MidiOutput,
	MidiInput,
	None = -1
};

class Device {
public:

	Device();
	Device(DeviceType type, const string &name, const string &internal_name, int channels);
	Device(DeviceType type, const Any& a);

	Any to_config() const;
	string get_name() const;
	bool is_default() const;

	string name;
	string internal_name;

	DeviceType type;
	int channels;
	bool visible;
	bool present;
	int client, port;
	int index_in_lib;
	bool default_by_lib;

	bool present_old;
};

}

#endif /* SRC_DEVICE_DEVICE_H_ */
