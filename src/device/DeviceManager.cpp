/*
 * DeviceManager.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "DeviceManager.h"
#include "backend-pulseaudio/DeviceContextPulse.h"

#include "../Session.h"
#include "../module/Module.h"
#include "Device.h"
#include "../Tsunami.h"
#include "../stuff/Log.h"

#if HAS_LIB_PORTAUDIO
#include <portaudio.h>
#endif

#if HAS_LIB_ALSA
#include <alsa/asoundlib.h>
#endif


struct ApiDescription {
	string name;
	DeviceManager::ApiType type;
	int mode;
	bool available;
};
Array<ApiDescription> api_descriptions = {
	{"alsa", DeviceManager::ApiType::ALSA, 2, HAS_LIB_ALSA},
	{"pulseaudio", DeviceManager::ApiType::PULSE, 1, HAS_LIB_PULSEAUDIO},
	{"portaudio", DeviceManager::ApiType::PORTAUDIO, 1, HAS_LIB_PORTAUDIO},
	{"-none-", DeviceManager::ApiType::NONE, 3, true}
};



// for legacy migration
Array<Device*> str2devs(const string &s, DeviceType type) {
	Array<Device *> devices;
	auto a = s.explode("|");
	for (string &b: a) {
		auto d = new Device(type, {});
		auto c = b.explode(",");
			if (c.num >= 4) {
			d->name = c[0].replace("${COMMA}", ",").replace("${PIPE}", "|");
			d->internal_name = c[1].replace("${COMMA}", ",").replace("${PIPE}", "|");
			d->channels = c[2]._int();
			d->visible = c[3]._bool();
		}
		devices.add(d);
	}
	return devices;
}

Array<Device*> parse_devs(const Any& a, DeviceType type) {
	Array<Device*> devices;
	if (a.is_array())
		for (const auto& aa: a.as_array())
			devices.add(new Device(type, aa));
	return devices;
}

Any devs2any(const Array<Device*>& devices) {
	Any a = Any::EmptyArray;
	for (auto d: devices)
		a.add(d->to_config());
	return a;
}


DeviceManager::DeviceManager(Session *_session) {
	initialized = false;

	session = _session;

	audio_api = ApiType::NONE;
	midi_api = ApiType::NONE;

#if HAS_LIB_ALSA
	alsa_midi_handle = nullptr;
#endif

	dummy_device = new Device(DeviceType::NONE, "dummy");

	msg_index = -1;
	msg_type = DeviceType::AUDIO_INPUT;
	output_volume = 0;
	hui_rep_id = -1;
}

DeviceManager::~DeviceManager() {
	if (hui_rep_id >= 0)
		hui::cancel_runner(hui_rep_id);

	for (Device *d: output_devices)
		delete d;
	for (Device *d: input_devices)
		delete d;
	for (Device *d: midi_input_devices)
		delete d;

	delete dummy_device;

	kill_library();
}

void DeviceManager::lock() {
	audio_context->lock();
}

void DeviceManager::unlock() {
	audio_context->unlock();
}

bool DeviceManager::audio_api_initialized() const {
	if (audio_context)
		return audio_context->fully_initialized;
#if HAS_LIB_PORTAUDIO
	if (audio_api == ApiType::PORTAUDIO)
		return portaudio_fully_initialized;
#endif
	return false;
}

void DeviceManager::remove_device(DeviceType type, int index) {
	auto &devices = device_list(type);
	if ((index < 0) or (index >= devices.num))
		return;
	if (devices[index]->present)
		return;
	delete devices[index];
	devices.erase(index);

	write_config();
	msg_type = type;
	msg_index = index;
	out_remove_device.notify();
}

void DeviceManager::write_config() {
	string audio_api_name = api_descriptions[(int)audio_api].name;
	string midi_api_name = api_descriptions[(int)midi_api].name;

	//hui::Config.set_float("Devices.OutputVolume", output_volume);
	hui::config.set("Devices.Output[" + audio_api_name + "]", devs2any(output_devices));
	hui::config.set("Devices.Input[" + audio_api_name + "]", devs2any(input_devices));
	hui::config.set("Devices.MidiInput[" + midi_api_name + "]", devs2any(midi_input_devices));
}


// don't poll pulse too much... it will send notifications anyways
void DeviceManager::update_devices(bool serious) {
	audio_context->update_device(this, serious);
#if HAS_LIB_PORTAUDIO
	if (audio_api == ApiType::PORTAUDIO) {
		_update_devices_audio_portaudio();
	}
#endif
#if HAS_LIB_ALSA
	if (midi_api == ApiType::ALSA)
		_update_devices_midi_alsa();
#endif

	write_config();
	out_changed.notify();
}

#if HAS_LIB_PORTAUDIO
void _portaudio_add_dev(DeviceManager *dm, DeviceType type, int index) {
	if (index < 0)
		return;
	const PaDeviceInfo* dev = Pa_GetDeviceInfo(index);
	if (!dev)
		return;
	int channels = (type == DeviceType::AUDIO_OUTPUT) ? dev->maxOutputChannels : dev->maxInputChannels;
	if (channels > 0) {
		Device *d = dm->get_device_create(type, string(Pa_GetHostApiInfo(dev->hostApi)->name) + "/" + dev->name);
		d->name = string(Pa_GetHostApiInfo(dev->hostApi)->name) + "/" + dev->name;
		d->channels = channels;
		d->index_in_lib = index;
		if (type == DeviceType::AUDIO_OUTPUT)
			d->default_by_lib = (index == Pa_GetDefaultOutputDevice());
		else
			d->default_by_lib = (index == Pa_GetDefaultInputDevice());
		d->present = true;
		dm->set_device_config(d);
	}
}
#endif

#if HAS_LIB_PORTAUDIO
void DeviceManager::_update_devices_audio_portaudio() {
	if (!portaudio_fully_initialized)
		return;
	for (Device *d: output_devices)
		d->present = false;
	for (Device *d: input_devices)
		d->present = false;

	// make sure, the default is first...
	_portaudio_add_dev(this, DeviceType::AUDIO_OUTPUT, Pa_GetDefaultOutputDevice());
	_portaudio_add_dev(this, DeviceType::AUDIO_INPUT, Pa_GetDefaultInputDevice());

	int count = Pa_GetDeviceCount();
	for (int i=0; i<count; i++) {
		_portaudio_add_dev(this, DeviceType::AUDIO_OUTPUT, i);
		_portaudio_add_dev(this, DeviceType::AUDIO_INPUT, i);
	}
}
#endif

#if HAS_LIB_ALSA
void DeviceManager::_update_devices_midi_alsa() {
	if (!alsa_midi_handle)
		return;

	for (Device *d: midi_input_devices) {
		d->present_old = d->present;
		d->present = false;
	}

	// default
	auto *def = get_device_create(DeviceType::MIDI_INPUT, "");
	def->default_by_lib = true;
	def->present = true;

	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;

	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_alloca(&pinfo);
	snd_seq_client_info_set_client(cinfo, -1);
	while (snd_seq_query_next_client(alsa_midi_handle, cinfo) >= 0) {
		snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
		snd_seq_port_info_set_port(pinfo, -1);
		while (snd_seq_query_next_port(alsa_midi_handle, pinfo) >= 0) {
			if ((snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_READ) == 0)
				continue;
			if ((snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_SUBS_READ) == 0)
				continue;
			Device *d = get_device_create(DeviceType::MIDI_INPUT, format("%s/%s", snd_seq_client_info_get_name(cinfo), snd_seq_port_info_get_name(pinfo)));
			d->name = d->internal_name;
			d->client = snd_seq_client_info_get_client(cinfo);
			d->port = snd_seq_port_info_get_port(pinfo);
			d->present = true;
		}
	}


	bool changed = false;
	for (Device *d: midi_input_devices)
		if (d->present_old != d->present)
			changed = true;
	if (changed)
		out_changed.notify();
}
#endif


static int select_api(const string &preferred_name, int mode) {
	int best = -1;
	for (int i=(int)DeviceManager::ApiType::NUM_APIS-1; i>=0; i--) {
		ApiDescription &a = api_descriptions[i];
		if (!a.available or ((a.mode & mode) == 0))
			continue;
		if (a.name == preferred_name)
			return i;
		best = i;//a.index;
	}
	return best;
}

void migrate_dev_config(const string& key, DeviceType type) {
	auto a = hui::config.get(key);
	if (a.is_string()) {
		const auto devs = str2devs(a.as_string(), type);
		hui::config.set(key, devs2any(devs));
	}
}

void DeviceManager::init() {
	if (initialized)
		return;

	hui::config.migrate("AudioApi", "Devices.AudioApi");
	hui::config.migrate("MidiApi", "Devices.MidiApi");
	for (auto& d: api_descriptions) {
		hui::config.migrate("Output.Devices[" + d.name + "]", "Devices.Output[" + d.name + "]");
		hui::config.migrate("Input.Devices[" + d.name + "]", "Devices.Input[" + d.name + "]");
		hui::config.migrate("MidiInput.Devices[" + d.name + "]", "Devices.MidiInput[" + d.name + "]");
		migrate_dev_config("Devices.Output[" + d.name + "]", DeviceType::AUDIO_OUTPUT);
		migrate_dev_config("Devices.Input[" + d.name + "]", DeviceType::AUDIO_INPUT);
		migrate_dev_config("Devices.MidiInput[" + d.name + "]", DeviceType::MIDI_INPUT);
	}
	hui::config.migrate("Output.Volume", "Devices.OutputVolume");

	audio_api = (ApiType)select_api(hui::config.get_str("Devices.AudioApi", "porteaudio"), 1);
	string audio_api_name = api_descriptions[(int)audio_api].name;
	session->i(_("audio library selected: ") + audio_api_name);
	midi_api = (ApiType)select_api(hui::config.get_str("Devices.MidiApi", "alsa"), 2);
	string midi_api_name = api_descriptions[(int)midi_api].name;
	session->i(_("midi library selected: ") + midi_api_name);

	hui::config.set_str("Devices.AudioApi", audio_api_name);
	hui::config.set_str("Devices.MidiApi", midi_api_name);



	output_devices = parse_devs(hui::config.get("Devices.Output[" + audio_api_name + "]"), DeviceType::AUDIO_OUTPUT);
	input_devices = parse_devs(hui::config.get("Devices.Input[" + audio_api_name + "]"), DeviceType::AUDIO_INPUT);
	midi_input_devices = parse_devs(hui::config.get("Devices.MidiInput[" + midi_api_name + "]"), DeviceType::MIDI_INPUT);

	output_volume = 1;
	//output_volume = hui::Config.get_float("Devices.OutputVolume", 1.0f);

	// audio

#if HAS_LIB_PULSEAUDIO
	if (audio_api == ApiType::PULSE) {
		audio_context = new DeviceContextPulse(session);
	}
#endif
	audio_context->out_request_update >> create_sink([this] {
		update_devices(true);
	});
	audio_context->out_device_found >> create_data_sink<Device>([this] (const Device& dd) {
		Device *d = get_device_create(dd.type, dd.internal_name);
		d->name = dd.name;
		d->channels = dd.channels;
		d->present = true;
		set_device_config(d);
	});

	audio_context->fully_initialized = audio_context->init();

#if HAS_LIB_PORTAUDIO
	if (audio_api == ApiType::PORTAUDIO)
		portaudio_fully_initialized = _init_audio_portaudio();
#endif


	// midi
#if HAS_LIB_ALSA
	if (midi_api == ApiType::ALSA)
		_init_midi_alsa();
#endif

	update_devices(true);


#if HAS_LIB_ALSA
	// only updating alsa makes sense...
	// pulse sends notifications and portaudio does not refresh internally (-_-)'
	hui_rep_id = hui::run_repeated(2.0f, [this] { _update_devices_midi_alsa(); });
#endif

	initialized = true;
}

#if HAS_LIB_PORTAUDIO
bool DeviceManager::_init_audio_portaudio() {
	PaError err = Pa_Initialize();
	_portaudio_test_error(err, session, "Pa_Initialize");
	if (err != paNoError)
		return false;

	session->i(_("please note, that portaudio does not support refreshing the device list after program launch"));
	return true;
}
#endif

#if HAS_LIB_ALSA
bool DeviceManager::_init_midi_alsa() {
	int r = snd_seq_open(&alsa_midi_handle, "hw", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
	if (r < 0) {
		session->e(string("Error opening ALSA sequencer: ") + snd_strerror(r));
		return false;
	}

	snd_seq_set_client_name(alsa_midi_handle, "Tsunami");
	return true;
}
#endif

void DeviceManager::kill_library() {
	if (!initialized)
		return;

	// audio
	if (audio_context) {
		delete audio_context;
		audio_context = nullptr;
	}

#if HAS_LIB_PORTAUDIO
	if (audio_api == ApiType::PORTAUDIO) {
		portaudio_fully_initialized = false;
		PaError err = Pa_Terminate();
		_portaudio_test_error(err, session, "Pa_Terminate");
	}
#endif

	// midi
#if HAS_LIB_ALSA
	if (alsa_midi_handle)
		snd_seq_close(alsa_midi_handle);
#endif

	initialized = false;
}


float DeviceManager::get_output_volume() {
	return output_volume;
}

void DeviceManager::set_output_volume(float _volume) {
	output_volume = _volume;
	write_config();
	out_changed.notify();
}

Device* DeviceManager::get_device(DeviceType type, const string &internal_name) {
	auto &devices = device_list(type);
	for (Device *d: devices)
		if (d->internal_name == internal_name)
			return d;
	return nullptr;
}

Device* DeviceManager::get_device_create(DeviceType type, const string &internal_name) {
	auto &devices = device_list(type);
	for (Device *d: devices)
		if (d->internal_name == internal_name)
			return d;
	Device *d = new Device(type, "", internal_name, 0);
	devices.add(d);
	msg_type = type;
	msg_index = devices.num - 1;
	out_add_device.notify();
	return d;
}

Array<Device*> &DeviceManager::device_list(DeviceType type) {
	if (type == DeviceType::AUDIO_OUTPUT)
		return output_devices;
	if (type == DeviceType::AUDIO_INPUT)
		return input_devices;
	if (type == DeviceType::MIDI_INPUT)
		return midi_input_devices;
	return empty_device_list;
}

Array<Device*> DeviceManager::good_device_list(DeviceType type) {
	auto &all = device_list(type);
	Array<Device*> list;
	for (Device *d: all)
		if (d->visible and d->present)
			list.add(d);
	return list;
}

Device *DeviceManager::choose_device(DeviceType type) {
	auto &devices = device_list(type);
	for (Device *d: devices)
		if (d->present and d->visible)
			return d;

	// unusable ...but not NULL
	return dummy_device;
}

void DeviceManager::set_device_config(Device *d) {
	/*Device *dd = get_device(d.type, d.internal_name);
	if (dd) {
		dd->name = d.name;
		dd->present = d.present;
		dd->visible = d.visible;
		dd->latency = d.latency;
		dd->client = d.client;
		dd->port = d.port;
		dd->channels = d.channels;
	} else {
		getDeviceList(d.type).add(d);
	}*/
	write_config();
	out_changed.notify();
}

void DeviceManager::make_device_top_priority(Device *d) {
	auto &devices = device_list(d->type);
	for (int i=0; i<devices.num; i++)
		if (devices[i] == d) {
			devices.insert(d, 0);
			devices.erase(i + 1);
			break;
		}
	write_config();
	out_changed.notify();
}

void DeviceManager::move_device_priority(Device *d, int new_prio) {
	auto &devices = device_list(d->type);
	for (int i=0; i<devices.num; i++)
		if (devices[i] == d) {
			devices.move(i, new_prio);
			break;
		}
	write_config();
	out_changed.notify();
}

#if HAS_LIB_PORTAUDIO
bool DeviceManager::_portaudio_test_error(PaError err, Session *session, const string &msg) {
	if (err != paNoError) {
		session->e(msg + ": " + Pa_GetErrorText(err));
		return true;
	}
	return false;
}
#endif
