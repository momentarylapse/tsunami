/*
 * DeviceConsole.cpp
 *
 *  Created on: 12.04.2016
 *      Author: michi
 */

#include "DeviceConsole.h"
#include "../../Session.h"
#include "../../Device/Device.h"
#include "../../Device/DeviceManager.h"

DeviceConsole::DeviceConsole(Session *session) :
	BottomBar::Console(_("Devices"), session)
{
	device_manager = session->device_manager;


	from_resource("device-manager");

	event_x("output-list", "hui:change", [=]{ on_output_edit(); });
	event_x("input-list", "hui:change", [=]{ on_input_edit(); });
	event_x("midi-input-list", "hui:change", [=]{ on_midi_input_edit(); });
	event_x("output-list", "hui:move", [=]{ on_output_move(); });
	event_x("input-list", "hui:move", [=]{ on_input_move(); });
	event_x("midi-input-list", "hui:move", [=]{ on_midi_input_move(); });
	event("top-priority", [=]{ on_top_priority(); });
	event("erase", [=]{ on_erase(); });

	device_manager->subscribe(this, [=]{ add_device(); }, device_manager->MESSAGE_ADD_DEVICE);
	device_manager->subscribe(this, [=]{ update_full(); }, device_manager->MESSAGE_REMOVE_DEVICE);
	device_manager->subscribe(this, [=]{ change_data(); }, device_manager->MESSAGE_CHANGE);

	update_full();
}

DeviceConsole::~DeviceConsole()
{
	device_manager->unsubscribe(this);
}

string DeviceConsole::to_format(int i, const Device *d)
{
	if (d->type == DeviceType::AUDIO_OUTPUT)
		return format("%d\\%s\\%s\\%d\\%s\\%s", i + 1, d->get_name().c_str(), d->internal_name.c_str(), d->channels, d->visible ? "true" : "false", d->present ? "true" : "false");
	if (d->type == DeviceType::AUDIO_INPUT)
		return format("%d\\%s\\%s\\%d\\%s\\%s\\%.1f", i + 1, d->get_name().c_str(), d->internal_name.c_str(), d->channels, d->visible ? "true" : "false", d->present ? "true" : "false", d->latency);
	if (d->type == DeviceType::MIDI_INPUT)
		return format("%d\\%s\\%s\\%s\\%s", i + 1, d->get_name().c_str(), d->internal_name.c_str(), d->visible ? "true" : "false", d->present ? "true" : "false");
	return "";
}

void DeviceConsole::change_data()
{
	output_devices = device_manager->device_list(DeviceType::AUDIO_OUTPUT);
	foreachi(Device *d, output_devices, i)
		change_string("output-list", i, to_format(i, d));

	input_devices = device_manager->device_list(DeviceType::AUDIO_INPUT);
	foreachi(Device *d, input_devices, i)
		change_string("input-list", i, to_format(i, d));

	midi_input_devices = device_manager->device_list(DeviceType::MIDI_INPUT);
	foreachi(Device *d, midi_input_devices, i)
		change_string("midi-input-list", i, to_format(i, d));
}

void DeviceConsole::update_full()
{
	Device *sel_out = nullptr, *sel_in = nullptr, *sel_midi_in = nullptr;

	if (get_int("output-list") >= 0)
		sel_out = output_devices[get_int("output-list")];
	if (get_int("input-list") >= 0)
		sel_in = input_devices[get_int("input-list")];
	if (get_int("midi-input-list") >= 0)
		sel_midi_in = midi_input_devices[get_int("midi-input-list")];

	reset("output-list");
	output_devices = device_manager->device_list(DeviceType::AUDIO_OUTPUT);
	foreachi(Device *d, output_devices, i){
		add_string("output-list", to_format(i, d));
		if (d == sel_out)
			set_int("output-list", i);
	}

	reset("input-list");
	input_devices = device_manager->device_list(DeviceType::AUDIO_INPUT);
	foreachi(Device *d, input_devices, i){
		add_string("input-list", to_format(i, d));
		if (d == sel_in)
			set_int("input-list", i);
	}

	reset("midi-input-list");
	midi_input_devices = device_manager->device_list(DeviceType::MIDI_INPUT);
	foreachi(Device *d, midi_input_devices, i){
		add_string("midi-input-list", to_format(i, d));
		if (d == sel_midi_in)
			set_int("midi-input-list", i);
	}

}

void DeviceConsole::add_device()
{
	Array<Device*> &devices = device_manager->device_list(device_manager->msg_type);
	if (device_manager->msg_type == DeviceType::AUDIO_OUTPUT){
		output_devices = devices;
		Device *d = output_devices.back();
		add_string("output-list", to_format(devices.num - 1, d));
	}else if (device_manager->msg_type == DeviceType::AUDIO_INPUT){
		input_devices = devices;
		Device *d = input_devices.back();
		add_string("input-list", to_format(devices.num - 1, d));
	}else if (device_manager->msg_type == DeviceType::MIDI_INPUT){
		midi_input_devices = devices;
		Device *d = midi_input_devices.back();
		add_string("midi-input-list", to_format(devices.num - 1, d));
	}

}

void DeviceConsole::on_output_edit()
{
	auto e = hui::GetEvent();
	int index = e->row;
	int col = e->column;
	if (col == 4){
		output_devices[index]->visible = get_cell("", index, col)._bool();
	}
	device_manager->set_device_config(output_devices[index]);
}

void DeviceConsole::on_input_edit()
{
	auto e = hui::GetEvent();
	int index = e->row;
	int col = e->column;
	if (col == 4){
		input_devices[index]->visible = get_cell("", index, col)._bool();
	}else if (col == 6){
		input_devices[index]->latency = get_cell("", index, col)._float();
	}
	device_manager->set_device_config(input_devices[index]);
}

void DeviceConsole::on_midi_input_edit()
{
	auto e = hui::GetEvent();
	int index = e->row;
	int col = e->column;
	if (col == 3){
		midi_input_devices[index]->visible = get_cell("", index, col)._bool();
	}
	device_manager->set_device_config(midi_input_devices[index]);
}

void DeviceConsole::on_top_priority()
{
	int t = get_int("dev-tab");
	if (t == 0){
		int n = get_int("output-list");
		if (n >= 0)
			device_manager->make_device_top_priority(output_devices[n]);
	}else if (t == 1){
		int n = get_int("input-list");
		if (n >= 0)
			device_manager->make_device_top_priority(input_devices[n]);
	}else if (t == 2){
		int n = get_int("midi-input-list");
		if (n >= 0)
			device_manager->make_device_top_priority(midi_input_devices[n]);
	}
}


void DeviceConsole::on_output_move()
{
	auto e = hui::GetEvent();
	int source = e->row;
	int target = e->row_target;
	//msg_write(format("  move  %d  ->  %d", source, target));
	device_manager->move_device_priority(output_devices[source], target);
	set_int("output-list", target);
}

void DeviceConsole::on_input_move()
{
	auto e = hui::GetEvent();
	int source = e->row;
	int target = e->row_target;
	device_manager->move_device_priority(input_devices[source], target);
	set_int("input-list", target);
}

void DeviceConsole::on_midi_input_move()
{
	auto e = hui::GetEvent();
	int source = e->row;
	int target = e->row_target;
	device_manager->move_device_priority(midi_input_devices[source], target);
	set_int("midi-input-list", target);
}

void DeviceConsole::on_erase()
{
	int t = get_int("dev-tab");
	if (t == 0){
		int n = get_int("output-list");
		if (n >= 0)
			device_manager->remove_device(DeviceType::AUDIO_OUTPUT, n);
	}else if (t == 1){
		int n = get_int("input-list");
		if (n >= 0)
			device_manager->remove_device(DeviceType::AUDIO_INPUT, n);
	}else if (t == 2){
		int n = get_int("midi-input-list");
		if (n >= 0)
			device_manager->remove_device(DeviceType::MIDI_INPUT, n);
	}
}
