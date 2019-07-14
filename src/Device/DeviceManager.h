/*
 * DeviceManager.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_DEVICE_DEVICEMANAGER_H_
#define SRC_DEVICE_DEVICEMANAGER_H_

#include "../Stuff/Observable.h"
#include "../lib/base/base.h"
#include "../lib/hui/hui.h"


class AudioOutput;
class AudioInput;
class MidiInput;
class Module;
class Device;
enum class DeviceType;
class Session;

#if HAS_LIB_PULSEAUDIO
struct pa_context;
struct pa_threaded_mainloop;
#endif

#if HAS_LIB_PORTAUDIO
typedef int PaError;
#endif


#if HAS_LIB_ALSA
struct _snd_seq;
#endif

class DeviceManager : public Observable<VirtualBase> {
public:
	friend class AudioOutput;
	friend class AudioInput;
	friend class MidiInput;

	static const string MESSAGE_ADD_DEVICE;
	static const string MESSAGE_REMOVE_DEVICE;

	DeviceManager(Session *session);
	virtual ~DeviceManager();

	Session *session;

	void init();
	void kill();

	void _init_audio_pulse();
	void _init_audio_portaudio();
	void _init_midi_alsa();

	enum class ApiType {
		ALSA,
		PULSE,
		PORTAUDIO,
		NONE,
		NUM_APIS
	};
	ApiType audio_api;
	ApiType midi_api;
	
	void lock();
	void unlock();


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

#if HAS_LIB_PULSEAUDIO
	pa_context *pulse_context;
	pa_threaded_mainloop *pulse_mainloop;
	bool _pulse_test_error(Session *session, const string &msg);
#endif
#if HAS_LIB_PORTAUDIO
	static bool _portaudio_test_error(PaError err, Session *session, const string &msg);
#endif

#if HAS_LIB_ALSA
	_snd_seq *alsa_midi_handle;
#endif

	float output_volume;

	void update_devices(bool serious);
	void _update_devices_midi_alsa();
	void _update_devices_audio_pulse();
	void _update_devices_audio_portaudio();
	Array<Device*> empty_device_list;
	Array<Device*> output_devices;
	Array<Device*> input_devices;
	Array<Device*> midi_input_devices;
	Device *dummy_device;

	void write_config();

public:
	DeviceType msg_type;
	int msg_index;
};

#endif /* SRC_DEVICE_DEVICEMANAGER_H_ */
