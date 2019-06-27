/*
 * SynthConsole.h
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#ifndef SYNTHCONSOLE_H_
#define SYNTHCONSOLE_H_

#include "SideBar.h"

class Track;
class ModulePanel;

class SynthConsole : public SideBarConsole {
public:
	SynthConsole(Session *session);
	~SynthConsole() override;

	void clear();
	void set_track(Track *t);

	void on_select();
	void on_detune();

	void on_edit_song();
	void on_edit_track();

	void on_track_delete();
	void on_track_change();
	void on_view_cur_track_change();

	string id_inner;

	ModulePanel *panel;

	Track *track;
};

#endif /* SYNTHCONSOLE_H_ */
