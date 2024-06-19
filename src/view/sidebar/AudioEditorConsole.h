/*
 * AudioEditorConsole.h
 *
 *  Created on: 29.07.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_AUDIOEDITORCONSOLE_H
#define SRC_VIEW_SIDEBAR_AUDIOEDITORCONSOLE_H

#include "SideBar.h"

namespace tsunami {

class TrackLayer;

class AudioEditorConsole : public SideBarConsole {
public:
	AudioEditorConsole(Session *session, SideBar *bar);
	void on_enter() override;
	void on_leave() override;

	void on_layer_delete();
	void on_view_cur_layer_change();

	void on_edit_mode(int mode);
	void on_action_source();
	void on_action_effect();
	void on_action_volume();
	void on_action_scale();

	void clear();
	void set_layer(TrackLayer *t);

	void update();


	string id_inner;

	TrackLayer *layer;
};

}

#endif // SRC_VIEW_SIDEBAR_AUDIOEDITORCONSOLE_H
