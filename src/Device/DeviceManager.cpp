/*
 * DeviceManager.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "DeviceManager.h"

#include "../Session.h"
#include "../Module/Module.h"
#include "Device.h"
#include "../Tsunami.h"
#include "../Stuff/Log.h"

#if HAS_LIB_PULSEAUDIO
#include <pulse/pulseaudio.h>
#endif

#if HAS_LIB_PORTAUDIO
#include <portaudio.h>
#endif

#if HAS_LIB_ALSA
#include <alsa/asoundlib.h>
#endif

const string DeviceManager::MESSAGE_ADD_DEVICE = "AddDevice";
const string DeviceManager::MESSAGE_REMOVE_DEVICE = "RemoveDevice";


struct ApiDescription
{
	string name;
	DeviceManager::ApiType type;
	int mode;
	bool available;
};
ApiDescription api_descriptions[] = {
	{"alsa", DeviceManager::ApiType::ALSA, 2, HAS_LIB_ALSA},
	{"pulseaudio", DeviceManager::ApiType::PULSE, 1, HAS_LIB_PULSEAUDIO},
	{"portaudio", DeviceManager::ApiType::PORTAUDIO, 1, HAS_LIB_PORTAUDIO},
	{"-none-", DeviceManager::ApiType::NONE, 3, true}
};




#if HAS_LIB_PULSEAUDIO

void pulse_wait_op(Session *session, pa_operation *op)
{
	if (!op){
		session->e("pulse_wait_op:  op=nil");
		return;
	}
	//printf("-w-");
	int n = 0;
	//msg_write("wait op " + p2s(op));
	while (pa_operation_get_state(op) == PA_OPERATION_RUNNING){
		//printf(".");
		// PA_OPERATION_RUNNING
		//pa_mainloop_iterate(m, 1, NULL);
		n ++;
		if (n > 300)
			break;
		hui::Sleep(0.010f);
	}
	auto status = pa_operation_get_state(op);
	//printf("%d\n", status);
	if (status != PA_OPERATION_DONE){
		if (status == PA_OPERATION_RUNNING)
			session->e("pulse_wait_op() failed: still running");
		else if (status == PA_OPERATION_CANCELLED)
			session->e("pulse_wait_op() failed: cancelled");
		else
			session->e("pulse_wait_op() failed: ???");
	}
	pa_operation_unref(op);
	//msg_write(" ok");
//	printf("-o-");
}

void pulse_ignore_op(Session *session, pa_operation *op)
{
	if (!op){
		session->e("pulse_ignore_op:  op=nil");
		return;
	}
	pa_operation_unref(op);
}

void pulse_subscription_callback(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata)
{
	//msg_write(format("event  %d  %d", (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK), (t & PA_SUBSCRIPTION_EVENT_TYPE_MASK)));

	DeviceManager *out = (DeviceManager*)userdata;

	if (((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_NEW) or ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)){
		//printf("----change   %d\n", idx);

		hui::RunLater(0.1f, [out]{ out->update_devices(true); });
	}
}


bool pulse_wait_context_ready(pa_context *c)
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


void pulse_sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
	if (eol > 0 or !i or !userdata)
		return;

	//printf("output  %s ||  %s   %d   %d\n", i->name, i->description, i->index, i->channel_map.channels);

	DeviceManager *dm = (DeviceManager*)userdata;
	Device *d = dm->get_device_create(DeviceType::AUDIO_OUTPUT, i->name);
	d->name = i->description;
	d->channels = i->channel_map.channels;
	d->present = true;
	dm->set_device_config(d);
}

void pulse_source_info_callback(pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
	if (eol > 0 or !i or !userdata)
		return;

	//printf("input  %s ||  %s   %d   %d\n", i->name, i->description, i->index, i->channel_map.channels);

	DeviceManager *dm = (DeviceManager*)userdata;
	Device *d = dm->get_device_create(DeviceType::AUDIO_INPUT, i->name);
	d->name = i->description;
	d->channels = i->channel_map.channels;
	d->present = true;
	dm->set_device_config(d);
}

#endif


Array<Device*> str2devs(const string &s, DeviceType type)
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

	audio_api = ApiType::NONE;
	midi_api = ApiType::NONE;

#if HAS_LIB_PULSEAUDIO
	pulse_context = nullptr;
#endif

#if HAS_LIB_ALSA
	alsa_midi_handle = nullptr;
#endif

	dummy_device = new Device((DeviceType)-1, "dummy");

	init();

	// only updating alsa makes sense...
	// pulse sends notifications and portaudio does not refresh internally (-_-)'
	hui_rep_id = hui::RunRepeated(2.0f, [=]{ _update_devices_midi_alsa(); });
}

DeviceManager::~DeviceManager()
{
	hui::CancelRunner(hui_rep_id);

	write_config();
	kill();

	for (Device *d: output_devices)
		delete d;
	for (Device *d: input_devices)
		delete d;
	for (Device *d: midi_input_devices)
		delete d;

	delete dummy_device;
}

void DeviceManager::remove_device(DeviceType type, int index)
{
	Array<Device*> &devices = device_list(type);
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
	string audio_api_name = api_descriptions[(int)audio_api].name;
	string midi_api_name = api_descriptions[(int)midi_api].name;

	//hui::Config.set_str("Output.ChosenDevice", chosen_device);
	hui::Config.set_float("Output.Volume", output_volume);
	hui::Config.set_str("Output.Devices[" + audio_api_name + "]", devs2str(output_devices));
	hui::Config.set_str("Input.Devices[" + audio_api_name + "]", devs2str(input_devices));
	hui::Config.set_str("MidiInput.Devices[" + midi_api_name + "]", devs2str(midi_input_devices));
}


// don't poll pulse too much... it will send notifications anyways
void DeviceManager::update_devices(bool serious)
{
	if (audio_api == ApiType::PULSE){
		if (serious)
			_update_devices_audio_pulse();
	}else if (audio_api == ApiType::PORTAUDIO){
		_update_devices_audio_portaudio();
	}

	if (midi_api == ApiType::ALSA)
		_update_devices_midi_alsa();

	notify(MESSAGE_CHANGE);
	write_config();
}


void DeviceManager::_update_devices_audio_pulse()
{
	Session *session = Session::GLOBAL;

#if HAS_LIB_PULSEAUDIO

	for (Device *d: output_devices)
		d->present = false;
	for (Device *d: input_devices)
		d->present = false;

	// system default
	auto *def = get_device_create(DeviceType::AUDIO_OUTPUT, "");
	def->channels = 2;
	def->default_by_lib = true;
	def->present = true;

	pa_operation *op = pa_context_get_sink_info_list(pulse_context, pulse_sink_info_callback, this);
	if (!_pulse_test_error(session, "pa_context_get_sink_info_list"))
		pulse_wait_op(session, op);

	// system default
	def = get_device_create(DeviceType::AUDIO_INPUT, "");
	def->channels = 2;
	def->default_by_lib = true;
	def->present = true;

	op = pa_context_get_source_info_list(pulse_context, pulse_source_info_callback, this);
	if (!_pulse_test_error(session, "pa_context_get_source_info_list"))
		pulse_wait_op(session, op);


#endif
}

#if HAS_LIB_PORTAUDIO
void _portaudio_add_dev(DeviceManager *dm, DeviceType type, int index)
{
	if (index < 0)
		return;
	const PaDeviceInfo* dev = Pa_GetDeviceInfo(index);
	if (!dev)
		return;
	int channels = (type == DeviceType::AUDIO_OUTPUT) ? dev->maxOutputChannels : dev->maxInputChannels;
	if (channels > 0){
		Device *d = dm->get_device_create(type, string(Pa_GetHostApiInfo(dev->hostApi)->name) + "/" + dev->name);
		d->name = dev->name;
		d->channels = min(channels, 2);
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

void DeviceManager::_update_devices_audio_portaudio()
{
#if HAS_LIB_PORTAUDIO
	for (Device *d: output_devices)
		d->present = false;
	for (Device *d: input_devices)
		d->present = false;

	_portaudio_add_dev(this, DeviceType::AUDIO_OUTPUT, Pa_GetDefaultOutputDevice());
	_portaudio_add_dev(this, DeviceType::AUDIO_INPUT, Pa_GetDefaultInputDevice());

	int count = Pa_GetDeviceCount();
	for (int i=0; i<count; i++){
		_portaudio_add_dev(this, DeviceType::AUDIO_OUTPUT, i);
		_portaudio_add_dev(this, DeviceType::AUDIO_INPUT, i);
	}
#endif
}

void DeviceManager::_update_devices_midi_alsa()
{
#if HAS_LIB_ALSA

	if (!alsa_midi_handle)
		return;

	for (Device *d: midi_input_devices){
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
	while (snd_seq_query_next_client(alsa_midi_handle, cinfo) >= 0){
		snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
		snd_seq_port_info_set_port(pinfo, -1);
		while (snd_seq_query_next_port(alsa_midi_handle, pinfo) >= 0){
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
		notify(MESSAGE_CHANGE);
#endif
}


static int select_api(const string &preferred_name, int mode)
{
	int best = -1;
	for (int i=(int)DeviceManager::ApiType::NUM_APIS-1; i>=0; i--){
		ApiDescription &a = api_descriptions[i];
		if (!a.available or ((a.mode & mode) == 0))
			continue;
		if (a.name == preferred_name)
			return i;
		best = i;//a.index;
	}
	return best;
}

void DeviceManager::init()
{
	if (initialized)
		return;

	Session *session = Session::GLOBAL;

	audio_api = (ApiType)select_api(hui::Config.get_str("AudioApi", "porteaudio"), 1);
	string audio_api_name = api_descriptions[(int)audio_api].name;
	session->i(_("audio library selected: ") + audio_api_name);
	midi_api = (ApiType)select_api(hui::Config.get_str("MidiApi", "alsa"), 2);
	string midi_api_name = api_descriptions[(int)midi_api].name;
	session->i(_("midi library selected: ") + midi_api_name);

	hui::Config.set_str("AudioApi", audio_api_name);
	hui::Config.set_str("MidiApi", midi_api_name);



	output_devices = str2devs(hui::Config.get_str("Output.Devices[" + audio_api_name + "]", ""), DeviceType::AUDIO_OUTPUT);
	input_devices = str2devs(hui::Config.get_str("Input.Devices[" + audio_api_name + "]", ""), DeviceType::AUDIO_INPUT);
	midi_input_devices = str2devs(hui::Config.get_str("MidiInput.Devices[" + midi_api_name + "]", ""), DeviceType::MIDI_INPUT);

	output_volume = hui::Config.get_float("Output.Volume", 1.0f);

	// audio
	if (audio_api == ApiType::PULSE)
		_init_audio_pulse();
	else if (audio_api == ApiType::PORTAUDIO)
		_init_audio_portaudio();


	// midi
	if (midi_api == ApiType::ALSA)
		_init_midi_alsa();

	update_devices(true);

	initialized = true;
}

void DeviceManager::_init_audio_pulse()
{
	Session *session = Session::GLOBAL;

#if HAS_LIB_PULSEAUDIO
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

	pulse_context = pa_context_new(mainloop_api, "tsunami");
	if (_pulse_test_error(session, "pa_context_new"))
		return;

	pa_context_connect(pulse_context, nullptr, (pa_context_flags_t)0, nullptr);
	if (_pulse_test_error(session, "pa_context_connect"))
		return;

	pa_threaded_mainloop_start(m);
	if (_pulse_test_error(session, "pa_threaded_mainloop_start"))
		return;

	if (!pulse_wait_context_ready(pulse_context)){
		session->e("pulse audio context does not turn 'ready'");
		return;
	}

	pa_context_set_subscribe_callback(pulse_context, &pulse_subscription_callback, this);
	_pulse_test_error(session, "pa_context_set_subscribe_callback");
	pa_context_subscribe(pulse_context, (pa_subscription_mask_t)(PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SOURCE), nullptr, this);
	_pulse_test_error(session, "pa_context_subscribe");
#endif
}

void DeviceManager::_init_audio_portaudio()
{
	Session *session = Session::GLOBAL;

#if HAS_LIB_PORTAUDIO
	PaError err = Pa_Initialize();
	_portaudio_test_error(err, session, "Pa_Initialize");

	session->i(_("please note, that portaudio does not support refreshing the device list after program launch"));
#endif
}

void DeviceManager::_init_midi_alsa()
{
	Session *session = Session::GLOBAL;

#if HAS_LIB_ALSA
	int r = snd_seq_open(&alsa_midi_handle, "hw", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
	if (r < 0){
		session->e(string("Error opening ALSA sequencer: ") + snd_strerror(r));
		return;
	}

	snd_seq_set_client_name(alsa_midi_handle, "Tsunami");
#endif
}

void DeviceManager::kill()
{
	if (!initialized)
		return;

	Array<Module*> to_del = streams;
	for (Module *s: to_del)
		delete s;

	// audio
#if HAS_LIB_PULSEAUDIO
	if (audio_api == ApiType::PULSE and pulse_context){
		pa_context_disconnect(pulse_context);
		_pulse_test_error(Session::GLOBAL, "pa_context_disconnect");
	}
#endif

#if HAS_LIB_PORTAUDIO
	if (audio_api == ApiType::PORTAUDIO){
		PaError err = Pa_Terminate();
		_portaudio_test_error(err, Session::GLOBAL, "Pa_Terminate");
	}
#endif

	// midi
#if HAS_LIB_ALSA
	if (alsa_midi_handle)
		snd_seq_close(alsa_midi_handle);
#endif

	initialized = false;
}


float DeviceManager::get_output_volume()
{
	return output_volume;
}

void DeviceManager::set_output_volume(float _volume)
{
	output_volume = _volume;
	notify(MESSAGE_CHANGE);
}

void DeviceManager::add_stream(Module *s)
{
	streams.add(s);
}

void DeviceManager::remove_stream(Module *s)
{
	for (int i=streams.num-1; i>=0; i--)
		if (streams[i] == s)
			streams.erase(i);
}

bool DeviceManager::stream_exists(Module *s)
{
	for (int i=streams.num-1; i>=0; i--)
		if (streams[i] == s)
			return true;
	return false;
}

Device* DeviceManager::get_device(DeviceType type, const string &internal_name)
{
	Array<Device*> &devices = device_list(type);
	for (Device *d: devices)
		if (d->internal_name == internal_name)
			return d;
	return nullptr;
}

Device* DeviceManager::get_device_create(DeviceType type, const string &internal_name)
{
	Array<Device*> &devices = device_list(type);
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

Array<Device*> &DeviceManager::device_list(DeviceType type)
{
	if (type == DeviceType::AUDIO_OUTPUT)
		return output_devices;
	if (type == DeviceType::AUDIO_INPUT)
		return input_devices;
	if (type == DeviceType::MIDI_INPUT)
		return midi_input_devices;
	return empty_device_list;
}

Array<Device*> DeviceManager::good_device_list(DeviceType type)
{
	Array<Device*> &all = device_list(type);
	Array<Device*> list;
	for (Device *d: all)
		if (d->visible and d->present)
			list.add(d);
	return list;
}

Device *DeviceManager::choose_device(DeviceType type)
{
	Array<Device*> &devices = device_list(type);
	for (Device *d: devices)
		if (d->present and d->visible)
			return d;

	// unusable ...but not NULL
	return dummy_device;
}

void DeviceManager::set_device_config(Device *d)
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

void DeviceManager::make_device_top_priority(Device *d)
{
	Array<Device*> &devices = device_list(d->type);
	for (int i=0; i<devices.num; i++)
		if (devices[i] == d){
			devices.insert(d, 0);
			devices.erase(i + 1);
			break;
		}
	write_config();
	notify(MESSAGE_CHANGE);
}

void DeviceManager::move_device_priority(Device *d, int new_prio)
{
	Array<Device*> &devices = device_list(d->type);
	for (int i=0; i<devices.num; i++)
		if (devices[i] == d){
			devices.move(i, new_prio);
			break;
		}
	write_config();
	notify(MESSAGE_CHANGE);
}

#if HAS_LIB_PULSEAUDIO
bool DeviceManager::_pulse_test_error(Session *session, const string &msg)
{
	int e = pa_context_errno(pulse_context);
	if (e != 0)
		session->e(msg + ": " + pa_strerror(e));
	return (e != 0);
}
#endif

#if HAS_LIB_PORTAUDIO
bool DeviceManager::_portaudio_test_error(PaError err, Session *session, const string &msg)
{
	if (err != paNoError){
		session->e(msg + ": " + Pa_GetErrorText(err));
		return true;
	}
	return false;
}
#endif
