/*
 * Device.h
 *
 *  Created on: 13.04.2016
 *      Author: michi
 */

#ifndef SRC_AUDIO_DEVICE_H_
#define SRC_AUDIO_DEVICE_H_

#include "../lib/base/base.h"


class Device
{
public:
	Device();
	Device(int type, const string &name, const string &internal_name, int channels);
	Device(int type, const string &s);

	string to_config();
	string get_name() const;
	bool is_default() const;

	string name;
	string internal_name;

	enum Type{
		AUDIO_OUTPUT,
		AUDIO_INPUT,
		MIDI_OUTPUT,
		MIDI_INPUT
	};

	int type;
	int channels;
	bool visible;
	bool present;
	float latency;
	int client, port;

	bool present_old;
};

#endif /* SRC_AUDIO_DEVICE_H_ */
