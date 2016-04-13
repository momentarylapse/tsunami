/*
 * DeviceConsole.h
 *
 *  Created on: 12.04.2016
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_DEVICECONSOLE_H_
#define SRC_VIEW_BOTTOMBAR_DEVICECONSOLE_H_

#include "BottomBar.h"

class DeviceManager;
class Device;

class DeviceConsole: public BottomBarConsole, public Observer
{
public:
	DeviceConsole(DeviceManager *device_manager);
	virtual ~DeviceConsole();

	void update_full();
	void change_data();
	void add_device();

	void onOutputEdit();
	void onInputEdit();

	virtual void onUpdate(Observable *o, const string &message);

	DeviceManager *device_manager;
	Array<Device> output_devices;
	Array<Device> input_devices;
};

#endif /* SRC_VIEW_BOTTOMBAR_DEVICECONSOLE_H_ */
