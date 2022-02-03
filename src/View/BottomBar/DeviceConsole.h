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
enum class DeviceType;

class DeviceConsole: public hui::Panel {
public:
	DeviceConsole(Session *session, hui::Panel *parent);
	virtual ~DeviceConsole();

	void update_full();
	void change_data();
	void on_add_device();

	void on_move_device(DeviceType type);
	void on_right_click_device(DeviceType type);
	void on_erase();

	void on_device_hide();

	string to_format(const Device *d);

	int fav_index(const Device *d);

	DeviceManager *device_manager;

	hui::Menu *popup;
	Device *popup_device;
	Session *session;
};

#endif /* SRC_VIEW_BOTTOMBAR_DEVICECONSOLE_H_ */
