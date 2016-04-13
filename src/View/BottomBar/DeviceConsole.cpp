/*
 * DeviceConsole.cpp
 *
 *  Created on: 12.04.2016
 *      Author: michi
 */

#include "DeviceConsole.h"
#include "../../Audio/DeviceManager.h"
#include "../../Audio/Device.h"

DeviceConsole::DeviceConsole(DeviceManager *_device_manager) :
	BottomBarConsole(_("Ger&ate")),
	Observer("DeviceConsole")
{
	device_manager = _device_manager;



	addTabControl(_("Output\\Input"), 0, 0, 0, 0, "dev_tab");
	setTarget("dev_tab", 0);
	//addGrid("", 0, 0, 2, 1, "grid");
	//setTarget("grid", 0);
	addListView(_("!format=tttCc\\Name\\Interner Name\\Kan&ale\\anzeigen\\pr&asent"), 0, 0, 0, 0, "output_list");
	setTarget("dev_tab", 1);
	addListView(_("!format=tttCcT\\Name\\Interner Name\\Kan&ale\\anzeigen\\pr&asent\\Latenz (ms)"), 0, 0, 0, 0, "input_list");

	eventX("output_list", "hui:change", this, &DeviceConsole::onOutputEdit);
	eventX("input_list", "hui:change", this, &DeviceConsole::onInputEdit);

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
	}else{
		//update_full();
	}
}

void DeviceConsole::change_data()
{
	output_devices = device_manager->getOutputDevices();
	foreachi(Device &d, output_devices, i)
		changeString("output_list", i, format("%s\\%s\\%d\\%s\\%s", d.name.c_str(), d.internal_name.c_str(), d.channels, d.visible ? "true" : "false", d.present ? "true" : "false"));

	input_devices = device_manager->getInputDevices();
	foreachi(Device &d, input_devices, i)
		changeString("input_list", i, format("%s\\%s\\%d\\%s\\%s\\%.1f", d.name.c_str(), d.internal_name.c_str(), d.channels, d.visible ? "true" : "false", d.present ? "true" : "false", d.latency));
}

void DeviceConsole::update_full()
{
	string sel_out, sel_in;

	if (getInt("output_list") >= 0)
		sel_out = output_devices[getInt("output_list")].internal_name;
	if (getInt("input_list") >= 0)
		sel_in = input_devices[getInt("input_list")].internal_name;

	reset("output_list");
	output_devices = device_manager->getOutputDevices();
	foreachi(Device &d, output_devices, i){
		addString("output_list", format("%s\\%s\\%d\\%s\\%s", d.name.c_str(), d.internal_name.c_str(), d.channels, d.visible ? "true" : "false", d.present ? "true" : "false"));
		if (d.internal_name == sel_out)
			setInt("output_list", i);
	}

	reset("input_list");
	input_devices = device_manager->getInputDevices();
	foreachi(Device &d, input_devices, i){
		addString("input_list", format("%s\\%s\\%d\\%s\\%s\\%.1f", d.name.c_str(), d.internal_name.c_str(), d.channels, d.visible ? "true" : "false", d.present ? "true" : "false", d.latency));
		if (d.internal_name == sel_in)
			setInt("input_list", i);
	}

}

void DeviceConsole::add_device()
{
	if (device_manager->msg_type == Device::TYPE_OUTPUT){
		output_devices = device_manager->getOutputDevices();
		Device d = output_devices.back();
		addString("output_list", format("%s\\%s\\%d\\%s\\%s", d.name.c_str(), d.internal_name.c_str(), d.channels, d.visible ? "true" : "false", d.present ? "true" : "false"));
	}else{
		input_devices = device_manager->getInputDevices();
		Device d = input_devices.back();
		addString("input_list", format("%s\\%s\\%d\\%s\\%s\\%.1f", d.name.c_str(), d.internal_name.c_str(), d.channels, d.visible ? "true" : "false", d.present ? "true" : "false", d.latency));
	}

}

void DeviceConsole::onOutputEdit()
{
	int index = HuiGetEvent()->row;
	int col = HuiGetEvent()->column;
	if (col == 3){
		output_devices[index].visible = getCell("", index, col)._bool();
	}
	device_manager->setDeviceConfig(output_devices[index]);
}

void DeviceConsole::onInputEdit()
{
	int index = HuiGetEvent()->row;
	int col = HuiGetEvent()->column;
	if (col == 3){
		input_devices[index].visible = getCell("", index, col)._bool();
	}else if (col == 5){
		input_devices[index].latency = getCell("", index, col)._float();
	}
	device_manager->setDeviceConfig(input_devices[index]);
}
