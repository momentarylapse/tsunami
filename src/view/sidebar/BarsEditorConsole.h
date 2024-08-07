/*
 * BarsEditorConsole.h
 *
 *  Created on: 22.10.2022
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_BARSEDITORCONSOLE_H
#define SRC_VIEW_SIDEBAR_BARSEDITORCONSOLE_H

#include "SideBar.h"

namespace tsunami {

class TrackLayer;

class BarsEditorConsole : public SideBarConsole {
public:
	BarsEditorConsole(Session *session, SideBar *bar);

	void on_enter() override;
	void on_leave() override;

	void on_layer_delete();
	void on_view_cur_layer_change();

	void on_edit_mode(int mode);

	void on_action_edit_speed();
	void on_action_replace();

	void clear();
	void set_layer(TrackLayer *t);

	void update();


	string id_inner;

	TrackLayer *layer;
};

}

#endif // SRC_VIEW_SIDEBAR_BARSEDITORCONSOLE_H
