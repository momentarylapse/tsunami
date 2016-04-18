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
class Device;

struct pa_context;

class DeviceManager : public Observable
{
public:
	friend class AudioStream;
	friend class AudioInputAudio;

	static const string MESSAGE_ADD_DEVICE;
	static const string MESSAGE_REMOVE_DEVICE;

	DeviceManager();
	virtual ~DeviceManager();

	void init();
	void kill();


	float getOutputVolume();
	void setOutputVolume(float volume);

	void addStream(AudioStream *s);
	void removeStream(AudioStream *s);
	bool streamExists(AudioStream *s);

	void remove_device(int type, int index);

	void setDeviceConfig(Device *d);
	void makeDeviceTopPriority(Device *d);

	Device *get_device(int type, const string &internal_name);
	Device *get_device_create(int type, const string &internal_name);
	Array<Device*> &getDeviceList(int type);
	Array<Device*> getGoodDeviceList(int type);

	Device *chooseDevice(int type);

//private:
	bool testError(const string &msg);

	bool initialized;
	pa_context *context;

	float output_volume;

	Array<AudioStream*> streams;

	void update_devices();
	Array<Device*> empty_device_list;
	Array<Device*> output_devices;
	Array<Device*> input_devices;
	Array<Device*> midi_input_devices;

	void write_config();

public:
	int msg_type, msg_index;
};

#endif /* DEVICEMANAGER_H_ */
