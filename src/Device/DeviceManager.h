/*
 * DeviceManager.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef DEVICEMANAGER_H_
#define DEVICEMANAGER_H_

#include "../Stuff/Observable.h"
#include "../lib/base/base.h"
#include "../lib/hui/hui.h"


class OutputStream;
class InputStreamAudio;
class Device;
class Session;

#if HAS_LIB_PULSEAUDIO
struct pa_context;
#endif

#if HAS_LIB_ALSA
struct _snd_seq;
#endif

class DeviceManager : public Observable<VirtualBase>
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

	enum{
		API_ALSA,
		API_PULSE,
		API_PORTAUDIO
	};
	int api;


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
	bool testError(Session *session, const string &msg);

	bool initialized;
	int hui_rep_id;

#if HAS_LIB_PULSEAUDIO
	pa_context *pulse_context;
#endif

#if HAS_LIB_ALSA
	_snd_seq *alsa_midi_handle;
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
