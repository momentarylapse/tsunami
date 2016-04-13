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

	subscribe(device_manager, device_manager->MESSAGE_CHANGE_DEVICES);

	update();
}

DeviceConsole::~DeviceConsole()
{
	unsubscribe(device_manager);
}

void DeviceConsole::onUpdate(Observable *o, const string &message)
{
	update();
}

void DeviceConsole::update()
{
	reset("output_list");
	Array<Device> devices = device_manager->getOutputDevices();
	foreach(Device &d, devices)
		addString("output_list", format("%s\\%s\\%d\\%s\\%s", d.name.c_str(), d.internal_name.c_str(), d.channels, d.hidden ? "false" : "true", d.present ? "true" : "false"));

	reset("input_list");
	devices = device_manager->getInputDevices();
	foreach(Device &d, devices)
		addString("input_list", format("%s\\%s\\%d\\%s\\%s\\%.1f", d.name.c_str(), d.internal_name.c_str(), d.channels, d.hidden ? "false" : "true", d.present ? "true" : "false", d.latency));

}

