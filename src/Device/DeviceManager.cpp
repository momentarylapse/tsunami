/*
 * DeviceManager.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "../Session.h"
#include "../Tsunami.h"
#include "../Stuff/Log.h"
#include "Device.h"
#include "DeviceManager.h"
#include "OutputStream.h"

#ifdef DEVICE_PULSEAUDIO
#include <pulse/pulseaudio.h>
#endif

#ifdef DEVICE_PORTAUDIO
#include <portaudio.h>
#pragma comment(lib,"portaudio_x86.lib")
#endif

#ifdef DEVICE_MIDI_ALSA
#include <alsa/asoundlib.h>
#endif

const string DeviceManager::MESSAGE_ADD_DEVICE = "AddDevice";
const string DeviceManager::MESSAGE_REMOVE_DEVICE = "RemoveDevice";


#ifdef DEVICE_PULSEAUDIO

void pa_wait_op(Session *session, pa_operation *op)
{
	if (!op){
		session->e("pa_wait_op:  op=nil");
		return;
	}
//	printf("-w-");
	int n = 0;
	//msg_write("wait op " + p2s(op));
	while (pa_operation_get_state(op) != PA_OPERATION_DONE){
//		printf(".");
		// PA_OPERATION_RUNNING
		//pa_mainloop_iterate(m, 1, NULL);
		if (pa_operation_get_state(op) == PA_OPERATION_CANCELLED)
			break;
		n ++;
		if (n > 200)
			break;
		hui::Sleep(0.005f);
	}
	if (pa_operation_get_state(op) != PA_OPERATION_DONE)
		session->e("pa_wait_op: failed");
	pa_operation_unref(op);
	//msg_write(" ok");
//	printf("-o-");
}

void pa_subscription_callback(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata)
{
	//msg_write(format("event  %d  %d", (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK), (t & PA_SUBSCRIPTION_EVENT_TYPE_MASK)));

	DeviceManager *out = (DeviceManager*)userdata;

	if (((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_NEW) or ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)){
		//printf("----change   %d\n", idx);

		hui::RunLater(0.1f, std::bind(&DeviceManager::update_devices, out));
	}
}


bool pa_wait_context_ready(pa_context *c)
{
	//msg_write("wait stream ready");
	int n = 0;
	while (pa_context_get_state(c) != PA_CONTEXT_READY){
		//pa_mainloop_iterate(m, 1, NULL);
		hui::Sleep(0.01f);
		n ++;
		if (n >= 500)
			return false;
		if (pa_context_get_state(c) == PA_CONTEXT_FAILED)
			return false;
	}
	//msg_write("ok");
	return true;
}


void pa_sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
	if (eol > 0 or !i or !userdata)
		return;

	//printf("output  %s ||  %s   %d   %d\n", i->name, i->description, i->index, i->channel_map.channels);

	DeviceManager *dm = (DeviceManager*)userdata;
	Device *d = dm->get_device_create(Device::Type::AUDIO_OUTPUT, i->name);
	d->name = i->description;
	d->channels = i->channel_map.channels;
	d->present = true;
	dm->setDeviceConfig(d);
}

void pa_source_info_callback(pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
	if (eol > 0 or !i or !userdata)
		return;

	//printf("input  %s ||  %s   %d   %d\n", i->name, i->description, i->index, i->channel_map.channels);

	DeviceManager *dm = (DeviceManager*)userdata;
	Device *d = dm->get_device_create(Device::Type::AUDIO_INPUT, i->name);
	d->name = i->description;
	d->channels = i->channel_map.channels;
	d->present = true;
	dm->setDeviceConfig(d);
}

#endif


Array<Device*> str2devs(const string &s, int type)
{
	Array<Device*> devices;
	Array<string> a = s.explode("|");
	for (string &b: a)
		devices.add(new Device(type, b));
	return devices;
}

string devs2str(Array<Device*> devices)
{
	string r;
	foreachi(Device *d, devices, i){
		if (i > 0)
			r += "|";
		r += d->to_config();
	}
	return r;
}


DeviceManager::DeviceManager()
{
	initialized = false;

	output_devices = str2devs(hui::Config.getStr("Output.Devices", ""), Device::Type::AUDIO_OUTPUT);
	input_devices = str2devs(hui::Config.getStr("Input.Devices", ""), Device::Type::AUDIO_INPUT);
	midi_input_devices = str2devs(hui::Config.getStr("MidiInput.Devices", ""), Device::Type::MIDI_INPUT);

	output_volume = hui::Config.getFloat("Output.Volume", 1.0f);


#ifdef DEVICE_PULSEAUDIO
	context = NULL;
#endif

#ifdef DEVICE_MIDI_ALSA
	handle = NULL;
#endif

	// system defaults
	default_devices[Device::Type::AUDIO_OUTPUT] = get_device_create(Device::Type::AUDIO_OUTPUT, ":default:");
	default_devices[Device::Type::AUDIO_OUTPUT]->channels = 2;
	default_devices[Device::Type::AUDIO_INPUT] = get_device_create(Device::Type::AUDIO_INPUT, ":default:");
	default_devices[Device::Type::AUDIO_INPUT]->channels = 2;
	default_devices[Device::Type::MIDI_INPUT] = get_device_create(Device::Type::MIDI_INPUT, ":default:");

	init();

	hui_rep_id = hui::RunRepeated(2.0f, std::bind(&DeviceManager::update_midi_devices, this));
}

DeviceManager::~DeviceManager()
{
	hui::CancelRunner(hui_rep_id);

	write_config();
	kill();

	for (Device *d: output_devices)
		delete(d);
	for (Device *d: input_devices)
		delete(d);
	for (Device *d: midi_input_devices)
		delete(d);
}

void DeviceManager::remove_device(int type, int index)
{
	Array<Device*> &devices = getDeviceList(type);
	if ((index < 0) or (index >= devices.num))
		return;
	if (devices[index]->present)
		return;
	delete(devices[index]);
	devices.erase(index);

	write_config();
	msg_type = type;
	msg_index = index;
	notify(MESSAGE_REMOVE_DEVICE);
}

void DeviceManager::write_config()
{
	//hui::Config.setStr("Output.ChosenDevice", chosen_device);
	hui::Config.setFloat("Output.Volume", output_volume);
	hui::Config.setStr("Output.Devices", devs2str(output_devices));
	hui::Config.setStr("Input.Devices", devs2str(input_devices));
	hui::Config.setStr("MidiInput.Devices", devs2str(midi_input_devices));
}


void DeviceManager::update_devices()
{
	Session *session = Session::GLOBAL;

	for (Device *d: output_devices)
		d->present = false;
	for (Device *d: input_devices)
		d->present = false;

#ifdef DEVICE_PULSEAUDIO

	pa_operation *op = pa_context_get_sink_info_list(context, pa_sink_info_callback, this);
	if (!testError(session, "pa_context_get_sink_info_list"))
		pa_wait_op(session, op);

	default_devices[Device::Type::AUDIO_OUTPUT]->present = true;


	op = pa_context_get_source_info_list(context, pa_source_info_callback, this);
	if (!testError(session, "pa_context_get_source_info_list"))
		pa_wait_op(session, op);


	default_devices[Device::Type::AUDIO_INPUT]->present = true;


	update_midi_devices();

	notify(MESSAGE_CHANGE);
	write_config();
#endif
#ifdef DEVICE_PORTAUDIO
	/*int count = Pa_GetHostApiCount();
	for (int i=0; i<count; i++){
		const PaHostApiInfo* api_info = Pa_GetHostApiInfo(i);
		msg_write(api_info->name);
		msg_write(api_info->deviceCount);
	}*/
	int count = Pa_GetDeviceCount();
	for (int i=0; i<count; i++){
		const PaDeviceInfo* dev = Pa_GetDeviceInfo(i);
		if (dev->maxOutputChannels > 0){
			Device *d = get_device_create(Device::Type::AUDIO_OUTPUT, string(Pa_GetHostApiInfo(dev->hostApi)->name) + "/" + dev->name);
			d->name = dev->name;
			d->channels = min(dev->maxOutputChannels, 2);
			d->present = true;
			setDeviceConfig(d);
		}
		if (dev->maxInputChannels > 0){
			Device *d = get_device_create(Device::Type::AUDIO_INPUT, string(Pa_GetHostApiInfo(dev->hostApi)->name) + "/" + dev->name);
			d->name = dev->name;
			d->channels = min(dev->maxInputChannels, 2);
			d->present = true;
			setDeviceConfig(d);
		}
	}
#endif
}

void DeviceManager::update_midi_devices()
{
#ifdef DEVICE_MIDI_ALSA

	if (!handle)
		return;

	for (Device *d: midi_input_devices){
		d->present_old = d->present;
		d->present = false;
	}

	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;

	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_alloca(&pinfo);
	snd_seq_client_info_set_client(cinfo, -1);
	while (snd_seq_query_next_client(handle, cinfo) >= 0){
		snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
		snd_seq_port_info_set_port(pinfo, -1);
		while (snd_seq_query_next_port(handle, pinfo) >= 0){
			if ((snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_READ) == 0)
				continue;
			if ((snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_SUBS_READ) == 0)
				continue;
			Device *d = get_device_create(Device::Type::MIDI_INPUT, format("%s/%s", snd_seq_client_info_get_name(cinfo), snd_seq_port_info_get_name(pinfo)));
			d->name = d->internal_name;
			d->client = snd_seq_client_info_get_client(cinfo);
			d->port = snd_seq_port_info_get_port(pinfo);
			d->present = true;
		}
	}

	default_devices[Device::Type::MIDI_INPUT]->present = true;


	bool changed = false;
	for (Device *d: midi_input_devices)
		if (d->present_old != d->present)
			changed = true;
	if (changed)
		notify(MESSAGE_CHANGE);
#endif
}

void DeviceManager::init()
{
	if (initialized)
		return;

	Session *session = Session::GLOBAL;

	// audio
#ifdef DEVICE_PULSEAUDIO
	pa_threaded_mainloop* m = pa_threaded_mainloop_new();
	if (!m){
		session->e("pa_threaded_mainloop_new failed");
		return;
	}

	pa_mainloop_api *mainloop_api = pa_threaded_mainloop_get_api(m);
	if (!m){
		session->e("pa_threaded_mainloop_get_api failed");
		return;
	}

	context = pa_context_new(mainloop_api, "tsunami");
	if (testError(session, "pa_context_new"))
		return;

	pa_context_connect(context, NULL, (pa_context_flags_t)0, NULL);
	if (testError(session, "pa_context_connect"))
		return;

	pa_threaded_mainloop_start(m);
	if (testError(session, "pa_threaded_mainloop_start"))
		return;

	if (!pa_wait_context_ready(context)){
		session->e("pulse audio context does not turn 'ready'");
		return;
	}

	pa_context_set_subscribe_callback(context, &pa_subscription_callback, this);
	testError(session, "pa_context_set_subscribe_callback");
	pa_context_subscribe(context, (pa_subscription_mask_t)(PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SOURCE), NULL, this);
	testError(session, "pa_context_subscribe");
#endif

#ifdef DEVICE_PORTAUDIO
	Pa_Initialize();
#endif


	// midi
#ifdef DEVICE_MIDI_ALSA
	int r = snd_seq_open(&handle, "hw", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
	if (r < 0){
		session->e(string("Error opening ALSA sequencer: ") + snd_strerror(r));
		//return;
	}else{
	snd_seq_set_client_name(handle, "Tsunami");
	portid = snd_seq_create_simple_port(handle, "Tsunami MIDI in",
				SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
				SND_SEQ_PORT_TYPE_APPLICATION);
	if (portid < 0){
		session->e(string("Error creating sequencer port: ") + snd_strerror(portid));
		//return;
	}
	}
#endif

	update_devices();

	initialized = true;
}

void DeviceManager::kill()
{
	if (!initialized)
		return;

	Array<OutputStream*> to_del = streams;
	for (OutputStream *s: to_del)
		delete s;

	// audio
#ifdef DEVICE_PULSEAUDIO
	if (context){
		pa_context_disconnect(context);
		testError(Session::GLOBAL, "pa_context_disconnect");
	}
#endif

#ifdef DEVICE_PORTAUDIO
	Pa_Terminate();
#endif

	// midi
#ifdef DEVICE_MIDI_ALSA
	if (handle)
		snd_seq_close(handle);
#endif

	initialized = false;
}


float DeviceManager::getOutputVolume()
{
	return output_volume;
}

void DeviceManager::setOutputVolume(float _volume)
{
	output_volume = _volume;
	notify(MESSAGE_CHANGE);
}

void DeviceManager::addStream(OutputStream* s)
{
	streams.add(s);
}

void DeviceManager::removeStream(OutputStream* s)
{
	for (int i=streams.num-1; i>=0; i--)
		if (streams[i] == s)
			streams.erase(i);
}

bool DeviceManager::streamExists(OutputStream* s)
{
	for (int i=streams.num-1; i>=0; i--)
		if (streams[i] == s)
			return true;
	return false;
}

Device* DeviceManager::get_device(int type, const string &internal_name)
{
	Array<Device*> &devices = getDeviceList(type);
	for (Device *d: devices)
		if (d->internal_name == internal_name)
			return d;
	return NULL;
}

Device* DeviceManager::get_device_create(int type, const string &internal_name)
{
	Array<Device*> &devices = getDeviceList(type);
	for (Device *d: devices)
		if (d->internal_name == internal_name)
			return d;
	Device *d = new Device(type, "", internal_name, 0);
	devices.add(d);
	msg_type = type;
	msg_index = devices.num - 1;
	notify(MESSAGE_ADD_DEVICE);
	return d;
}

Array<Device*> &DeviceManager::getDeviceList(int type)
{
	if (type == Device::Type::AUDIO_OUTPUT)
		return output_devices;
	if (type == Device::Type::AUDIO_INPUT)
		return input_devices;
	if (type == Device::Type::MIDI_INPUT)
		return midi_input_devices;
	return empty_device_list;
}

Array<Device*> DeviceManager::getGoodDeviceList(int type)
{
	Array<Device*> &all = getDeviceList(type);
	Array<Device*> list;
	for (Device *d: all)
		if (d->visible and d->present)
			list.add(d);
	return list;
}

Device *DeviceManager::chooseDevice(int type)
{
	Array<Device*> &devices = getDeviceList(type);
	for (Device *d: devices)
		if (d->present and d->visible)
			return d;

	// unusable ...but not NULL
	return default_devices[type];
}

void DeviceManager::setDeviceConfig(Device *d)
{
	/*Device *dd = get_device(d.type, d.internal_name);
	if (dd){
		dd->name = d.name;
		dd->present = d.present;
		dd->visible = d.visible;
		dd->latency = d.latency;
		dd->client = d.client;
		dd->port = d.port;
		dd->channels = d.channels;
	}else{
		getDeviceList(d.type).add(d);
	}*/
	write_config();
	notify(MESSAGE_CHANGE);
}

void DeviceManager::makeDeviceTopPriority(Device *d)
{
	Array<Device*> &devices = getDeviceList(d->type);
	for (int i=0; i<devices.num; i++)
		if (devices[i] == d){
			devices.insert(d, 0);
			devices.erase(i + 1);
			break;
		}
	write_config();
	notify(MESSAGE_CHANGE);
}

void DeviceManager::moveDevicePriority(Device *d, int new_prio)
{
	Array<Device*> &devices = getDeviceList(d->type);
	for (int i=0; i<devices.num; i++)
		if (devices[i] == d){
			devices.move(i, new_prio);
			break;
		}
	write_config();
	notify(MESSAGE_CHANGE);
}

bool DeviceManager::testError(Session *session, const string &msg)
{
#ifdef DEVICE_PULSEAUDIO
	int e = pa_context_errno(context);
	if (e != 0)
		session->e(msg + ": " + pa_strerror(e));
	return (e != 0);
#endif
	return false;
}
