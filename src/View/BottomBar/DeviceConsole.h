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

class DeviceConsole: public BottomBar::Console
{
public:
	DeviceConsole(Session *session);
	virtual ~DeviceConsole();

	void update_full();
	void change_data();
	void add_device();

	void onOutputEdit();
	void onInputEdit();
	void onMidiInputEdit();
	void onOutputMove();
	void onInputMove();
	void onMidiInputMove();
	void onTopPriority();
	void onErase();

	string to_format(int i, const Device *d);

	DeviceManager *device_manager;
	Array<Device*> output_devices;
	Array<Device*> input_devices;
	Array<Device*> midi_input_devices;
};

#endif /* SRC_VIEW_BOTTOMBAR_DEVICECONSOLE_H_ */
