/*
 * SampleRefConsole.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SAMPLEREFCONSOLE_H_
#define SAMPLEREFCONSOLE_H_

#include "SideBar.h"
class Track;
class TrackLayer;
class SampleRef;

class SampleRefConsole: public SideBarConsole {
public:
	SampleRefConsole(Session *session, SideBar *bar);
	void on_enter() override;
	void on_leave() override;

	void load_data();
	void apply_data();

	void on_name();
	void on_mute();
	void on_track();
	void on_volume();

	void on_view_cur_sample_change();
	void on_update();

	TrackLayer *layer;
	SampleRef *sample;

	bool editing;
};

#endif /* SAMPLEREFCONSOLE_H_ */
