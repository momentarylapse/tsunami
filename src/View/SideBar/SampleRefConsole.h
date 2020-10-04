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
	SampleRefConsole(Session *session);
	virtual ~SampleRefConsole();

	void load_data();
	void apply_data();

	void on_name();
	void on_mute();
	void on_track();
	void on_volume();

	void on_edit_song();
	void on_edit_track();
	void on_edit_sample();

	void on_view_cur_sample_change();
	void on_update();

	TrackLayer *layer;
	SampleRef *sample;
};

#endif /* SAMPLEREFCONSOLE_H_ */
