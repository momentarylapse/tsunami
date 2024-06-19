/*
 * MixingConsole.h
 *
 *  Created on: 16.03.2014
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_MIXINGCONSOLE_H_
#define SRC_VIEW_BOTTOMBAR_MIXINGCONSOLE_H_


#include "BottomBar.h"

namespace tsunami {


class PeakMeterDisplay;
class DeviceManager;

class TrackMixer;

class MixingConsole: public BottomBar::Console {
public:
	MixingConsole(Session *session, BottomBar *bar);
	virtual ~MixingConsole();

	void load_data();

	void on_output_volume();

	void on_tracks_change();
	void update_all();
	void on_update_device_manager();
	void on_chain_state_change();

	void _cdecl on_show() override;
	void _cdecl on_hide() override;

	DeviceManager *device_manager;
	owned<PeakMeterDisplay> peak_meter;
	owned<PeakMeterDisplay> spectrum_meter;

	string id_inner;
	shared_array<TrackMixer> mixer;

	int peak_runner_id;
	
	void show_fx(Track *t);
};

}

#endif /* SRC_VIEW_BOTTOMBAR_MIXINGCONSOLE_H_ */
