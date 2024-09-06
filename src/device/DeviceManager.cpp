/*
 * DeviceManager.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "DeviceManager.h"
#include "interface/DeviceContext.h"
#include "backend-pulseaudio/DeviceContextPulse.h"
#include "backend-portaudio/DeviceContextPort.h"
#include "backend-pipewire/DeviceContextPipewire.h"
#include "backend-alsa/DeviceContextAlsa.h"
#include "backend-coreaudio/DeviceContextCoreAudio.h"
#include "backend-coremidi/DeviceContextCoreMidi.h"
#include "Device.h"
#include "../Session.h"
#include "../lib/any/any.h"
#include "../lib/hui/hui.h"
#include "../lib/os/config.h"

namespace tsunami {

Array<DeviceManager::ApiDescription> DeviceManager::api_descriptions = {
	{"alsa", DeviceManager::ApiType::Alsa, 2, HAS_LIB_ALSA},
	{"pulseaudio", DeviceManager::ApiType::Pulseaudio, 1, HAS_LIB_PULSEAUDIO},
	{"portaudio", DeviceManager::ApiType::Portaudio, 1, HAS_LIB_PORTAUDIO},
	{"pipewire", DeviceManager::ApiType::Pipewire, 1, HAS_LIB_PIPEWIRE},
	{"coreaudio", DeviceManager::ApiType::Coreaudio, 1, HAS_LIB_COREAUDIO},
	{"coremidi", DeviceManager::ApiType::Coremidi, 2, HAS_LIB_COREMIDI},
	{"dummy", DeviceManager::ApiType::Dummy, 3, true}
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
	if (a.is_list())
		for (const auto& aa: a.as_list())
			devices.add(new Device(type, aa));
	return devices;
}

Any devs2any(const Array<Device*>& devices) {
	Any a = Any::EmptyList;
	for (auto d: devices)
		a.add(d->to_config());
	return a;
}


DeviceContext* create_backend_context(Session* session, DeviceManager::ApiType api) {
#if HAS_LIB_COREAUDIO
	if (api == DeviceManager::ApiType::Coreaudio)
		return new DeviceContextCoreAudio(session);
#endif
#if HAS_LIB_PULSEAUDIO
	if (api == DeviceManager::ApiType::Pulseaudio)
		return new DeviceContextPulse(session);
#endif
#if HAS_LIB_PORTAUDIO
	if (api == DeviceManager::ApiType::Portaudio)
		return new DeviceContextPort(session);
#endif
#if HAS_LIB_COREMIDI
	if (api == DeviceManager::ApiType::Coremidi)
		return new DeviceContextCoreMidi(session);
#endif
#if HAS_LIB_PIPEWIRE
	if (api == DeviceManager::ApiType::Pipewire)
		return new DeviceContextPipewire(session);
#endif
#if HAS_LIB_ALSA
	if (api == DeviceManager::ApiType::Alsa)
		return new DeviceContextAlsa(session);
#endif

	class DeviceContextDummy : public DeviceContext {
	public:
		DeviceContextDummy(Session* session) : DeviceContext(session) {}
		bool init(Session* session) override { return true; }
		void update_device(DeviceManager* device_manager, bool serious) override {}
	};
	return new DeviceContextDummy(session);
}


DeviceManager::DeviceManager(Session *_session) {
	initialized = false;

	session = _session;

	audio_api = ApiType::Dummy;
	midi_api = ApiType::Dummy;

	dummy_device = new Device(DeviceType::None, "dummy");

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

bool DeviceManager::audio_api_initialized() const {
	if (audio_context)
		return audio_context->fully_initialized;
	return false;
}

void DeviceManager::remove_device(DeviceType type, int index) {
	auto &devices = device_list(type);
	if ((index < 0) or (index >= devices.num))
		return;
	if (devices[index]->present)
		return;
	out_remove_device.notify(devices[index]);
	delete devices[index];
	devices.erase(index);

	write_config();
}

DeviceManager::ApiDescription find_api_description(DeviceManager::ApiType api) {
	for (auto &d: DeviceManager::api_descriptions)
		if (d.type == api)
			return d;
	return DeviceManager::api_descriptions.back();
}

void DeviceManager::write_config() {
	string audio_api_name = find_api_description(audio_api).name;
	string midi_api_name = find_api_description(midi_api).name;

	//hui::Config.set_float("Devices.OutputVolume", output_volume);
	hui::config.set("Devices.Output[" + audio_api_name + "]", devs2any(output_devices));
	hui::config.set("Devices.Input[" + audio_api_name + "]", devs2any(input_devices));
	hui::config.set("Devices.MidiInput[" + midi_api_name + "]", devs2any(midi_input_devices));
}


// don't poll pulse too much... it will send notifications anyways
void DeviceManager::update_devices(bool initial_discovery) {
	base::set<Device*> present_old;
	for (auto d: all_devices()) {
		if (d->present)
			present_old.add(d);
		d->present = false;
	}

	audio_context->update_device(this, initial_discovery);
	if (midi_context)
		midi_context->update_device(this, initial_discovery);


	for (auto d: all_devices()) {
		if (d->present and !present_old.contains(d)) {
			out_device_plugged_in(d);
			if (!initial_discovery)
				session->status(_("device plugged in: ") + d->get_name());
		} else if (!d->present and present_old.contains(d)) {
			out_device_plugged_out(d);
			if (!initial_discovery)
				session->status(_("device plugged out: ") + d->get_name());
		}
	}

	write_config();
	out_changed();
}


static DeviceManager::ApiType select_api(const string &preferred_name, int mode) {
	DeviceManager::ApiType best = DeviceManager::ApiType::Dummy;
	for (int i=(int)DeviceManager::ApiType::Count-1; i>=0; i--) {
		auto &a = DeviceManager::api_descriptions[i];
		if (!a.available or ((a.mode & mode) == 0))
			continue;
		if (a.name == preferred_name)
			return a.type;
		best = a.type;
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

static string suggest_default_audio_api() {
#if HAS_LIB_COREAUDIO
	return "coreaudio";
#elif HAS_LIB_PULSEAUDIO
	return "pulseaudio";
#elif HAS_LIB_PIPEWIRE
	return "pipewire";
#endif

	return "portaudio";
}

static string suggest_default_midi_api() {
#if HAS_LIB_COREMIDI
	return "coremidi";
#endif

#if HAS_LIB_ALSA
	return "alsa";
#endif

	return "";
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
		migrate_dev_config("Devices.Output[" + d.name + "]", DeviceType::AudioOutput);
		migrate_dev_config("Devices.Input[" + d.name + "]", DeviceType::AudioInput);
		migrate_dev_config("Devices.MidiInput[" + d.name + "]", DeviceType::MidiInput);
	}
	hui::config.migrate("Output.Volume", "Devices.OutputVolume");

	audio_api = select_api(hui::config.get_str("Devices.AudioApi", suggest_default_audio_api()), 1);
	string audio_api_name = find_api_description(audio_api).name;
	session->i(_("audio library selected: ") + audio_api_name);
	midi_api = (ApiType)select_api(hui::config.get_str("Devices.MidiApi", suggest_default_midi_api()), 2);
	string midi_api_name = find_api_description(midi_api).name;
	session->i(_("midi library selected: ") + midi_api_name);

	hui::config.set_str("Devices.AudioApi", audio_api_name);
	hui::config.set_str("Devices.MidiApi", midi_api_name);



	output_devices = parse_devs(hui::config.get("Devices.Output[" + audio_api_name + "]"), DeviceType::AudioOutput);
	input_devices = parse_devs(hui::config.get("Devices.Input[" + audio_api_name + "]"), DeviceType::AudioInput);
	midi_input_devices = parse_devs(hui::config.get("Devices.MidiInput[" + midi_api_name + "]"), DeviceType::MidiInput);

	output_volume = 1;
	//output_volume = hui::Config.get_float("Devices.OutputVolume", 1.0f);

	auto init_context = [this](DeviceContext* ctx) {
		ctx->out_request_update >> create_sink([this] {
			update_devices(false);
		});
		ctx->out_device_found >> create_data_sink<Device>([this] (const Device& dd) {
			//msg_write("DEVICE FOUND..." + dd.internal_name);
			Device *d = get_device_create(dd.type, dd.internal_name);
			*d = dd;
			d->present = true;
			set_device_config(d);
		});
		ctx->fully_initialized = ctx->init(session);
	};

	// audio
	audio_context = create_backend_context(session, audio_api);
	init_context(audio_context);

	// midi
	midi_context = create_backend_context(session, midi_api);
	init_context(midi_context);

	update_devices(true);


#if HAS_LIB_ALSA
	// only updating alsa makes sense...
	// pulse sends notifications and portaudio does not refresh internally (-_-)'
	if (midi_api == ApiType::Alsa)
		hui_rep_id = hui::run_repeated(2.0f, [this] {
			DeviceContextAlsa::instance->update_device(this, true);
		});
#endif

	initialized = true;
}


void DeviceManager::kill_library() {
	if (!initialized)
		return;

	// audio
	if (audio_context) {
		delete audio_context;
		audio_context = nullptr;
	}

	// midi
	if (midi_context) {
		delete midi_context;
		midi_context = nullptr;
	}

	initialized = false;
}


float DeviceManager::get_output_volume() {
	return output_volume;
}

void DeviceManager::set_output_volume(float _volume) {
	output_volume = _volume;
	write_config();
	out_changed();
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
	out_add_device.notify(d);
	return d;
}

Array<Device*> DeviceManager::all_devices() const {
	auto all = output_devices;
	all.append(input_devices);
	all.append(midi_input_devices);
	return all;
}

Array<Device*> &DeviceManager::device_list(DeviceType type) {
	if (type == DeviceType::AudioOutput)
		return output_devices;
	if (type == DeviceType::AudioInput)
		return input_devices;
	if (type == DeviceType::MidiInput)
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
	out_changed();
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
	out_changed();
}

void DeviceManager::move_device_priority(Device *d, int new_prio) {
	auto &devices = device_list(d->type);
	for (int i=0; i<devices.num; i++)
		if (devices[i] == d) {
			devices.move(i, new_prio);
			break;
		}
	write_config();
	out_changed();
}

}
