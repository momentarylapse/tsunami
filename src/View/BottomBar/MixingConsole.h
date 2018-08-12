/*
 * MixingConsole.h
 *
 *  Created on: 16.03.2014
 *      Author: michi
 */

#ifndef MIXINGCONSOLE_H_
#define MIXINGCONSOLE_H_


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

	void _cdecl onShow() override;
	void _cdecl onHide() override;

	DeviceManager *device_manager;
	PeakMeterDisplay *peak_meter;

	string id_inner;
	Array<TrackMixer*> mixer;
};

#endif /* MIXINGCONSOLE_H_ */
