/*
 * DeviceManager.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_DEVICE_DEVICEMANAGER_H_
#define SRC_DEVICE_DEVICEMANAGER_H_

#include "../lib/pattern/Observable.h"
#include "../lib/base/base.h"
#include "../lib/hui/hui.h"

namespace tsunami {

class AudioOutput;
class AudioInput;
class MidiInput;
class Module;
class Device;
enum class DeviceType;
class DeviceContext;
class Session;


class DeviceManager : public obs::Node<VirtualBase> {
public:
	friend class AudioOutput;
	friend class AudioInput;
	friend class MidiInput;

	obs::xsource<Device*> out_add_device{this, "add-device"};
	obs::xsource<Device*> out_remove_device{this, "remove-device"};

	DeviceManager(Session *session);
	virtual ~DeviceManager();

	Session *session;

	void init();
	void kill_library();

	enum class ApiType {
		ALSA,
		PULSE,
		PORTAUDIO,
		NONE,
		NUM_APIS
	};
	ApiType audio_api;
	ApiType midi_api;
	
	bool audio_api_initialized() const;


	float get_output_volume();
	void set_output_volume(float volume);

	void remove_device(DeviceType type, int index);

	void set_device_config(Device *d);
	void make_device_top_priority(Device *d);
	void move_device_priority(Device *d, int new_prio);

	Device *get_device(DeviceType type, const string &internal_name);
	Device *get_device_create(DeviceType type, const string &internal_name);
	Array<Device*> &device_list(DeviceType type);
	Array<Device*> good_device_list(DeviceType type);

	Device *choose_device(DeviceType type);

//private:

	bool initialized;
	int hui_rep_id;

	DeviceContext* audio_context = nullptr;
	DeviceContext* midi_context = nullptr;

	float output_volume;

	void update_devices(bool serious);
	Array<Device*> empty_device_list;
	Array<Device*> output_devices;
	Array<Device*> input_devices;
	Array<Device*> midi_input_devices;
	Device *dummy_device;

	void write_config();

	struct ApiDescription {
		string name;
		DeviceManager::ApiType type;
		int mode;
		bool available;
	};
	static Array<ApiDescription> api_descriptions;
};

}

#endif /* SRC_DEVICE_DEVICEMANAGER_H_ */
