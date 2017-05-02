/*
 * DeviceManager.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef DEVICEMANAGER_H_
#define DEVICEMANAGER_H_

#include "config.h"
#include "../Data/Song.h"
#include "../lib/base/base.h"
#include "../lib/hui/hui.h"


class OutputStream;
class InputStreamAudio;
class Device;

#ifdef DEVICE_PULSEAUDIO
struct pa_context;
#endif

#ifdef DEVICE_MIDI_ALSA
struct _snd_seq;
#endif

enum
{
	BACKUP_MODE_NONE,
	BACKUP_MODE_TEMP,
	BACKUP_MODE_KEEP
};

class DeviceManager : public Observable
{
public:
	friend class OutputStream;
	friend class InputStreamAudio;
	friend class InputStreamMidi;

	static const string MESSAGE_ADD_DEVICE;
	static const string MESSAGE_REMOVE_DEVICE;

	DeviceManager();
	virtual ~DeviceManager();

	void init();
	void kill();


	float getOutputVolume();
	void setOutputVolume(float volume);

	void addStream(OutputStream *s);
	void removeStream(OutputStream *s);
	bool streamExists(OutputStream *s);

	void remove_device(int type, int index);

	void setDeviceConfig(Device *d);
	void makeDeviceTopPriority(Device *d);
	void moveDevicePriority(Device *d, int new_prio);

	Device *get_device(int type, const string &internal_name);
	Device *get_device_create(int type, const string &internal_name);
	Array<Device*> &getDeviceList(int type);
	Array<Device*> getGoodDeviceList(int type);

	Device *chooseDevice(int type);

//private:
	bool testError(const string &msg);

	bool initialized;
	int hui_rep_id;

#ifdef DEVICE_PULSEAUDIO
	pa_context *context;
#endif

#ifdef DEVICE_MIDI_ALSA
	_snd_seq *handle;
#endif
	int portid;

	float output_volume;

	Array<OutputStream*> streams;

	void update_midi_devices();
	void update_devices();
	Array<Device*> empty_device_list;
	Array<Device*> output_devices;
	Array<Device*> input_devices;
	Array<Device*> midi_input_devices;
	Device* default_devices[4];

	void write_config();

public:
	int msg_type, msg_index;
};

#endif /* DEVICEMANAGER_H_ */
