/*
 * EffectsConsole.h
 *
 *  Created on: 22 May 2023
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_EFFECTSCONSOLE_H_
#define SRC_VIEW_SIDEBAR_EFFECTSCONSOLE_H_

#include "SideBar.h"

namespace tsunami {

class Track;
class ModulePanel;
class FxListEditor;

class EffectsConsole: public SideBarConsole {
public:
	EffectsConsole(Session *session, SideBar *bar);

	void on_enter() override;
	void on_leave() override;

	void load_data();

	void set_track(Track *t);

	void on_view_cur_track_change();
	void on_update();

	Track *track;
	owned<FxListEditor> fx_editor;
};

}

#endif /* SRC_VIEW_SIDEBAR_EFFECTSCONSOLE_H_ */
