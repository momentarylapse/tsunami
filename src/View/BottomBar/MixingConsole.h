/*
 * MixingConsole.h
 *
 *  Created on: 16.03.2014
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_MIXINGCONSOLE_H_
#define SRC_VIEW_BOTTOMBAR_MIXINGCONSOLE_H_


#include "BottomBar.h"

class PeakMeterDisplay;
class DeviceManager;

class TrackMixer;

class MixingConsole: public BottomBar::Console
{
public:
	MixingConsole(Session *session);
	virtual ~MixingConsole();

	void load_data();

	void on_output_volume();

	void on_tracks_change();
	void update_all();
	void on_update_device_manager();

	void _cdecl on_show() override;
	void _cdecl on_hide() override;

	DeviceManager *device_manager;
	PeakMeterDisplay *peak_meter;

	string id_inner;
	Array<TrackMixer*> mixer;
};

#endif /* SRC_VIEW_BOTTOMBAR_MIXINGCONSOLE_H_ */
