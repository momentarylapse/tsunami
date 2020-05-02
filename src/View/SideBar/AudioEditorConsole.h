/*
 * AudioEditorConsole.h
 *
 *  Created on: 29.07.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_AUDIOEDITORCONSOLE_H
#define SRC_VIEW_SIDEBAR_AUDIOEDITORCONSOLE_H

#include "SideBar.h"

class TrackLayer;

class AudioEditorConsole : public SideBarConsole {
public:
	AudioEditorConsole(Session *session);

	void on_layer_delete();
	void on_view_cur_layer_change();

	void on_edit_mode();

	void clear();
	void set_layer(TrackLayer *t);


	string id_inner;

	TrackLayer *layer;
};

#endif // SRC_VIEW_SIDEBAR_AUDIOEDITORCONSOLE_H
