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

	string name;
	string internal_name;

	enum{
		TYPE_OUTPUT,
		TYPE_INPUT
	};

	int type;
	int channels;
	bool visible;
	bool present;
	float latency;
};

#endif /* SRC_AUDIO_DEVICE_H_ */
