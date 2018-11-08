/*
 * MidiFxConsole.h
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#ifndef MIDIFXCONSOLE_H_
#define MIDIFXCONSOLE_H_

#include "SideBar.h"

class Track;

class MidiFxConsole : public SideBarConsole
{
public:
	MidiFxConsole(Session *session);
	virtual ~MidiFxConsole();

	void on_view_cur_track_change();
	void on_track_delete();
	void on_update();
	void update();

	void clear();
	void set_track(Track *t);

	void on_add();

	void on_edit_song();
	void on_edit_track();
	void on_edit_midi();

	string id_inner;

	Track *track;
	Array<hui::Panel*> panels;
};

#endif /* MIDIFXCONSOLE_H_ */
