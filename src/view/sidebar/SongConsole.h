/*
 * SongConsole.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SONGCONSOLE_H_
#define SONGCONSOLE_H_

#include "SideBar.h"
class Song;

class SongConsole: public SideBarConsole {
public:
	SongConsole(Session *session, SideBar *bar);
	void on_enter() override;
	void on_leave() override;

	void load_data();
	void apply_data();

	void on_samplerate();
	void on_format();
	void on_track_list();
	void on_tags_edit();
	void on_tags_right_click();
	void on_tag_add();
	void on_tag_delete();

	void on_update();
	
	owned<hui::Menu> menu_tags;
};

#endif /* SONGCONSOLE_H_ */
