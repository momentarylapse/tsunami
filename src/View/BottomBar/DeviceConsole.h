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

	void on_output_edit();
	void on_input_edit();
	void on_midi_input_edit();
	void on_output_move();
	void on_input_move();
	void on_midi_input_move();
	void on_top_priority();
	void on_erase();

	string to_format(const Device *d);

	int fav_index(const Device *d);

	DeviceManager *device_manager;
	Array<Device*> output_devices;
	Array<Device*> input_devices;
	Array<Device*> midi_input_devices;
};

#endif /* SRC_VIEW_BOTTOMBAR_DEVICECONSOLE_H_ */
