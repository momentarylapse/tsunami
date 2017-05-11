/*
 * DeviceConsole.cpp
 *
 *  Created on: 12.04.2016
 *      Author: michi
 */

#include "DeviceConsole.h"
#include "../../Device/DeviceManager.h"
#include "../../Device/Device.h"

DeviceConsole::DeviceConsole(DeviceManager *_device_manager) :
	BottomBarConsole(_("Devices")),
	Observer("DeviceConsole")
{
	device_manager = _device_manager;


	fromResource("device-manager");

	eventX("output-list", "hui:change", std::bind(&DeviceConsole::onOutputEdit, this));
	eventX("input-list", "hui:change", std::bind(&DeviceConsole::onInputEdit, this));
	eventX("midi-input-list", "hui:change", std::bind(&DeviceConsole::onMidiInputEdit, this));
	eventX("output-list", "hui:move", std::bind(&DeviceConsole::onOutputMove, this));
	eventX("input-list", "hui:move", std::bind(&DeviceConsole::onInputMove, this));
	eventX("midi-input-list", "hui:move", std::bind(&DeviceConsole::onMidiInputMove, this));
	event("top-priority", std::bind(&DeviceConsole::onTopPriority, this));
	event("erase", std::bind(&DeviceConsole::onErase, this));

	subscribe(device_manager);

	update_full();
}

DeviceConsole::~DeviceConsole()
{
	unsubscribe(device_manager);
}

void DeviceConsole::onUpdate(Observable *o, const string &message)
{
	if (message == device_manager->MESSAGE_CHANGE){
		change_data();
	}else if (message == device_manager->MESSAGE_ADD_DEVICE){
		add_device();
	}else if (message == device_manager->MESSAGE_REMOVE_DEVICE){
		update_full();
	}else{
	}
}

string DeviceConsole::to_format(int i, const Device *d)
{
	if (d->type == d->TYPE_AUDIO_OUTPUT)
		return format("%d\\%s\\%s\\%d\\%s\\%s", i + 1, d->get_name().c_str(), d->internal_name.c_str(), d->channels, d->visible ? "true" : "false", d->present ? "true" : "false");
	if (d->type == d->TYPE_AUDIO_INPUT)
		return format("%d\\%s\\%s\\%d\\%s\\%s\\%.1f", i + 1, d->get_name().c_str(), d->internal_name.c_str(), d->channels, d->visible ? "true" : "false", d->present ? "true" : "false", d->latency);
	if (d->type == d->TYPE_MIDI_INPUT)
		return format("%d\\%s\\%s\\%s\\%s", i + 1, d->get_name().c_str(), d->internal_name.c_str(), d->visible ? "true" : "false", d->present ? "true" : "false");
	return "";
}

void DeviceConsole::change_data()
{
	output_devices = device_manager->getDeviceList(Device::TYPE_AUDIO_OUTPUT);
	foreachi(Device *d, output_devices, i)
		changeString("output-list", i, to_format(i, d));

	input_devices = device_manager->getDeviceList(Device::TYPE_AUDIO_INPUT);
	foreachi(Device *d, input_devices, i)
		changeString("input-list", i, to_format(i, d));

	midi_input_devices = device_manager->getDeviceList(Device::TYPE_MIDI_INPUT);
	foreachi(Device *d, midi_input_devices, i)
		changeString("midi-input-list", i, to_format(i, d));
}

void DeviceConsole::update_full()
{
	Device *sel_out = NULL, *sel_in = NULL, *sel_midi_in = NULL;

	if (getInt("output-list") >= 0)
		sel_out = output_devices[getInt("output-list")];
	if (getInt("input-list") >= 0)
		sel_in = input_devices[getInt("input-list")];
	if (getInt("midi-input-list") >= 0)
		sel_midi_in = midi_input_devices[getInt("midi-input-list")];

	reset("output-list");
	output_devices = device_manager->getDeviceList(Device::TYPE_AUDIO_OUTPUT);
	foreachi(Device *d, output_devices, i){
		addString("output-list", to_format(i, d));
		if (d == sel_out)
			setInt("output-list", i);
	}

	reset("input-list");
	input_devices = device_manager->getDeviceList(Device::TYPE_AUDIO_INPUT);
	foreachi(Device *d, input_devices, i){
		addString("input-list", to_format(i, d));
		if (d == sel_in)
			setInt("input-list", i);
	}

	reset("midi-input-list");
	midi_input_devices = device_manager->getDeviceList(Device::TYPE_MIDI_INPUT);
	foreachi(Device *d, midi_input_devices, i){
		addString("midi-input-list", to_format(i, d));
		if (d == sel_midi_in)
			setInt("midi-input-list", i);
	}

}

void DeviceConsole::add_device()
{
	Array<Device*> &devices = device_manager->getDeviceList(device_manager->msg_type);
	if (device_manager->msg_type == Device::TYPE_AUDIO_OUTPUT){
		output_devices = devices;
		Device *d = output_devices.back();
		addString("output-list", to_format(devices.num - 1, d));
	}else if (device_manager->msg_type == Device::TYPE_AUDIO_INPUT){
		input_devices = devices;
		Device *d = input_devices.back();
		addString("input-list", to_format(devices.num - 1, d));
	}else if (device_manager->msg_type == Device::TYPE_MIDI_INPUT){
		midi_input_devices = devices;
		Device *d = midi_input_devices.back();
		addString("midi-input-list", to_format(devices.num - 1, d));
	}

}

void DeviceConsole::onOutputEdit()
{
	int index = HuiGetEvent()->row;
	int col = HuiGetEvent()->column;
	if (col == 4){
		output_devices[index]->visible = getCell("", index, col)._bool();
	}
	device_manager->setDeviceConfig(output_devices[index]);
}

void DeviceConsole::onInputEdit()
{
	int index = HuiGetEvent()->row;
	int col = HuiGetEvent()->column;
	if (col == 4){
		input_devices[index]->visible = getCell("", index, col)._bool();
	}else if (col == 6){
		input_devices[index]->latency = getCell("", index, col)._float();
	}
	device_manager->setDeviceConfig(input_devices[index]);
}

void DeviceConsole::onMidiInputEdit()
{
	int index = HuiGetEvent()->row;
	int col = HuiGetEvent()->column;
	if (col == 3){
		midi_input_devices[index]->visible = getCell("", index, col)._bool();
	}
	device_manager->setDeviceConfig(midi_input_devices[index]);
}

void DeviceConsole::onTopPriority()
{
	int t = getInt("dev-tab");
	if (t == 0){
		int n = getInt("output-list");
		if (n >= 0)
			device_manager->makeDeviceTopPriority(output_devices[n]);
	}else if (t == 1){
		int n = getInt("input-list");
		if (n >= 0)
			device_manager->makeDeviceTopPriority(input_devices[n]);
	}else if (t == 2){
		int n = getInt("midi-input-list");
		if (n >= 0)
			device_manager->makeDeviceTopPriority(midi_input_devices[n]);
	}
}


void DeviceConsole::onOutputMove()
{
	int source = HuiGetEvent()->row;
	int target = HuiGetEvent()->row_target;
	//msg_write(format("  move  %d  ->  %d", source, target));
	device_manager->moveDevicePriority(output_devices[source], target);
	setInt("output-list", target);
}

void DeviceConsole::onInputMove()
{
	int source = HuiGetEvent()->row;
	int target = HuiGetEvent()->row_target;
	device_manager->moveDevicePriority(input_devices[source], target);
	setInt("input-list", target);
}

void DeviceConsole::onMidiInputMove()
{
	int source = HuiGetEvent()->row;
	int target = HuiGetEvent()->row_target;
	device_manager->moveDevicePriority(midi_input_devices[source], target);
	setInt("midi-input-list", target);
}

void DeviceConsole::onErase()
{
	int t = getInt("dev-tab");
	if (t == 0){
		int n = getInt("output-list");
		if (n >= 0)
			device_manager->remove_device(Device::TYPE_AUDIO_OUTPUT, n);
	}else if (t == 1){
		int n = getInt("input-list");
		if (n >= 0)
			device_manager->remove_device(Device::TYPE_AUDIO_INPUT, n);
	}else if (t == 2){
		int n = getInt("midi-input-list");
		if (n >= 0)
			device_manager->remove_device(Device::TYPE_MIDI_INPUT, n);
	}
}
