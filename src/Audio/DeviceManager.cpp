/*
 * DeviceManager.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "AudioStream.h"
#include "../Tsunami.h"
#include "../Stuff/Log.h"

#include <pulse/pulseaudio.h>
#include "DeviceManager.h"

const string DeviceManager::MESSAGE_CHANGE_DEVICES = "ChangeDevices";


void pa_wait_op(pa_operation *op)
{
	if (!op){
		tsunami->log->error("pa_wait_op:  op=nil");
		return;
	}
	//msg_write("wait op " + p2s(op));
	while (pa_operation_get_state(op) != PA_OPERATION_DONE){
		// PA_OPERATION_RUNNING
		//pa_mainloop_iterate(m, 1, NULL);
		if (pa_operation_get_state(op) == PA_OPERATION_CANCELLED)
			break;
	}
	if (pa_operation_get_state(op) != PA_OPERATION_DONE)
		tsunami->log->error("pa_wait_op: failed");
	pa_operation_unref(op);
	//msg_write(" ok");
}

void pa_subscription_callback(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata)
{
	printf("----change   %d\n", idx);
	DeviceManager *out = (DeviceManager*)userdata;
	out->dirty = true;

	HuiRunLaterM(0.1f, out, &DeviceManager::SendDeviceChange);
	msg_write(format("event  %d  %d", (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK), (t & PA_SUBSCRIPTION_EVENT_TYPE_MASK)));
	/*if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SOURCE) {
		if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_NEW) {
		}
	}*/
}

Array<Device> str2devs(const string &s, int type)
{
	Array<Device> devices;
	Array<string> a = s.explode("||");
	foreach(string &b, a){
		Array<string> c = b.explode("|");
		if (c.num < 4)
			continue;
		Device d;
		d.type = type;
		d.name = c[0];
		d.internal_name = c[1];
		d.channels = c[2]._int();
		d.hidden = c[3]._bool();
		devices.add(d);
	}
	return devices;
}

string devs2str(Array<Device> devices)
{
	string r;
	foreachi(Device &d, devices, i){
		if (i > 0)
			r += "||";
		r += d.name + "|";
		r += d.internal_name + "|";
		r += i2s(d.channels) + "|";
		r += b2s(d.hidden);
	}
	return r;
}

DeviceManager::DeviceManager() :
	Observable("AudioOutput")
{
	initialized = false;
	dirty = true;

	output_devices = str2devs(HuiConfig.getStr("Output.Devices", ""), Device::TYPE_OUTPUT);
	input_devices = str2devs(HuiConfig.getStr("Input.Devices", ""), Device::TYPE_INPUT);

	chosen_device = HuiConfig.getStr("Output.ChosenDevice", "");
	output_volume = HuiConfig.getFloat("Output.Volume", 1.0f);
	//msg_write("chosen: " + chosen_device);

	init();
}

DeviceManager::~DeviceManager()
{
	kill();
	HuiConfig.setStr("Output.ChosenDevice", chosen_device);
	HuiConfig.setFloat("Output.Volume", output_volume);
}

void DeviceManager::setDevice(const string &device)
{
	HuiConfig.setStr("Output.ChosenDevice", device);

	Array<string> devs = getDevices();

	// valid?
	bool valid = (device == "");
	foreach(string &d, devs)
		if (d == device)
			valid = true;


	if (valid){
		chosen_device = device;
	}else{
		tsunami->log->error(format("output device '%s' not found. Using default.", device.c_str()));
		chosen_device = "";
	}

	//tsunami->log->info(format("output device '%s' chosen", Pa_GetDeviceInfo(pa_device_no)->name));
}



void pa_sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
	if (eol > 0 or !i or !userdata)
		return;


	//printf("output  %s ||  %s   %d   %d\n", i->name, i->description, i->index, i->channel_map.channels);

	Array<Device> *devices = (Array<Device>*)userdata;
	Device d;
	d.type = d.TYPE_OUTPUT;
	d.name = i->description;
	d.internal_name = i->name;
	d.channels = i->channel_map.channels;
	d.present = true;
	d.hidden = false;
	foreach(Device &dd, *devices){
		if (dd.name == d.name){
			dd.present = true;
			//dd = d;
			return;
		}
	}
	devices->add(d);
}

void pa_source_info_callback(pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
	if (eol > 0 or !i or !userdata)
		return;

	//printf("input  %s ||  %s   %d   %d\n", i->name, i->description, i->index, i->channel_map.channels);

	Array<Device> *devices = (Array<Device>*)userdata;
	Device d;
	d.type = d.TYPE_INPUT;
	d.name = i->description;
	d.internal_name = i->name;
	d.channels = i->channel_map.channels;
	d.present = true;
	d.hidden = false;
	foreach(Device &dd, *devices){
		if (dd.name == d.name){
			dd.present = true;
			//dd = d;
			return;
		}
	}
	devices->add(d);
}

Array<string> DeviceManager::getDevices()
{
	Array<string> devices;

	update_devices();

	foreach(Device &d, output_devices)
		devices.add(d.name);

	return devices;
}

void DeviceManager::update_devices()
{
	if (!dirty)
		return;

	foreach(Device &d, output_devices)
		d.present = false;

	pa_operation *op = pa_context_get_sink_info_list(context, pa_sink_info_callback, &output_devices);
	if (!testError("pa_context_get_sink_info_list"))
		pa_wait_op(op);

	foreach(Device &d, input_devices)
		d.present = false;

	op = pa_context_get_source_info_list(context, pa_source_info_callback, &input_devices);
	if (!testError("pa_context_get_source_info_list"))
		pa_wait_op(op);

	dirty = false;

	HuiConfig.setStr("Output.Devices", devs2str(output_devices));
	HuiConfig.setStr("Input.Devices", devs2str(input_devices));
}

Array<Device> DeviceManager::getOutputDevices()
{
	update_devices();
	return output_devices;
}

Array<Device> DeviceManager::getInputDevices()
{
	update_devices();
	return input_devices;
}


bool pa_wait_context_ready(pa_context *c)
{
	//msg_write("wait stream ready");
	int n = 0;
	while (pa_context_get_state(c) != PA_CONTEXT_READY){
		//pa_mainloop_iterate(m, 1, NULL);
		HuiSleep(0.01f);
		n ++;
		if (n >= 500)
			return false;
		if (pa_context_get_state(c) == PA_CONTEXT_FAILED)
			return false;
	}
	//msg_write("ok");
	return true;
}

void DeviceManager::init()
{
	if (initialized)
		return;
	msg_db_f("Output.init", 1);

	pa_threaded_mainloop* m = pa_threaded_mainloop_new();
	if (!m){
		tsunami->log->error("pa_threaded_mainloop_new failed");
		return;
	}

	pa_mainloop_api *mainloop_api = pa_threaded_mainloop_get_api(m);
	if (!m){
		tsunami->log->error("pa_threaded_mainloop_get_api failed");
		return;
	}

	context = pa_context_new(mainloop_api, "tsunami");
	if (testError("pa_context_new"))
		return;

	pa_context_connect(context, NULL, (pa_context_flags_t)0, NULL);
	if (testError("pa_context_connect"))
		return;

	pa_threaded_mainloop_start(m);
	if (testError("pa_threaded_mainloop_start"))
		return;

	if (!pa_wait_context_ready(context)){
		tsunami->log->error("pulse audio context does not turn 'ready'");
		return;
	}

	setDevice(chosen_device);

	pa_context_set_subscribe_callback(context, &pa_subscription_callback, this);
	testError("pa_context_set_subscribe_callback");
	pa_context_subscribe(context, (pa_subscription_mask_t)(PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SOURCE), NULL, this);
	testError("pa_context_subscribe");

	initialized = true;

	update_devices();
}

void DeviceManager::kill()
{
	if (!initialized)
		return;
	msg_db_f("Output.kill",1);

	foreach(AudioStream *s, streams)
		s->kill();

	pa_context_disconnect(context);
	testError("pa_context_disconnect");

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

void DeviceManager::addStream(AudioStream* s)
{
	streams.add(s);
}

void DeviceManager::removeStream(AudioStream* s)
{
	for (int i=streams.num-1; i>=0; i--)
		if (streams[i] == s)
			streams.erase(i);
}

bool DeviceManager::streamExists(AudioStream* s)
{
	for (int i=streams.num-1; i>=0; i--)
		if (streams[i] == s)
			return true;
	return false;
}

void DeviceManager::SendDeviceChange()
{
	notify(MESSAGE_CHANGE_DEVICES);
}

bool DeviceManager::testError(const string &msg)
{
	int e = pa_context_errno(context);
	if (e != 0)
		tsunami->log->error(msg + ": " + pa_strerror(e));
	return (e != 0);
}


