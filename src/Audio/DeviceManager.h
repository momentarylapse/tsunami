/*
 * DeviceManager.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef DEVICEMANAGER_H_
#define DEVICEMANAGER_H_

#include "../Data/Song.h"
#include "../lib/base/base.h"
#include "../lib/hui/hui.h"

class AudioStream;
class AudioInputAudio;
struct pa_context;


class Device
{
public:

	string name;
	string internal_name;

	enum{
		TYPE_OUTPUT,
		TYPE_INPUT
	};

	int type;
	int channels;
	bool hidden;
	bool present;
};

class DeviceManager : public Observable
{
public:
	friend class AudioStream;
	friend class AudioInputAudio;

	static const string MESSAGE_CHANGE_DEVICES;

	DeviceManager();
	virtual ~DeviceManager();


	Array<Device> getOutputDevices();
	Array<Device> getInputDevices();

	Array<string> getDevices();
	string chosen_device;
	void setDevice(const string &device);

	void init();
	void kill();


	float getOutputVolume();
	void setOutputVolume(float volume);

	void addStream(AudioStream *s);
	void removeStream(AudioStream *s);
	bool streamExists(AudioStream *s);

	void SendDeviceChange();

private:
	bool testError(const string &msg);

	bool initialized;
	pa_context *context;

	float output_volume;

	Array<AudioStream*> streams;

	void update_devices();
	Array<Device> output_devices;
	Array<Device> input_devices;

public:
	bool dirty;
};

#endif /* DEVICEMANAGER_H_ */
