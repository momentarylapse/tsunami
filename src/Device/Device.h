/*
 * Device.h
 *
 *  Created on: 13.04.2016
 *      Author: michi
 */

#ifndef SRC_DEVICE_DEVICE_H_
#define SRC_DEVICE_DEVICE_H_

#include "../lib/base/base.h"

enum class DeviceType{
	AUDIO_OUTPUT,
	AUDIO_INPUT,
	MIDI_OUTPUT,
	MIDI_INPUT
};

class Device
{
public:

	Device();
	Device(DeviceType type, const string &name, const string &internal_name, int channels);
	Device(DeviceType type, const string &s);

	string to_config();
	string get_name() const;
	bool is_default() const;

	string name;
	string internal_name;

	DeviceType type;
	int channels;
	bool visible;
	bool present;
	float latency;
	int client, port;
	int index_in_lib;
	bool default_by_lib;

	bool present_old;
};

#endif /* SRC_DEVICE_DEVICE_H_ */
