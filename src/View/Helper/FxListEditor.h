/*
 * FxListEditor.h
 *
 *  Created on: May 4, 2021
 *      Author: michi
 */

#pragma once

#include "../../lib/base/base.h"
#include "../../lib/base/pointer.h"

class Session;
class Track;
class Module;
class ModulePanel;
class AudioViewTrack;
namespace hui {
	class Panel;
	class Menu;
}


//todo set/unset track...
class FxListEditor : public VirtualBase {
public:
	FxListEditor(AudioViewTrack *t, hui::Panel *p, const string &_id, const string &_id_midi);
	~FxListEditor();
	hui::Panel *panel;
	string id_fx_list;
	string id_midi_fx_list;
	AudioViewTrack *vtrack;
	Module *selected_module = nullptr;
	owned<ModulePanel> config_panel;

	Session *session() const;
	Track *track() const;
	void select_module(Module *m);
	void on_fx_select();
	void on_fx_edit();
	void on_fx_move();
	owned<hui::Menu> menu_fx;
	void on_fx_right_click();
	void on_fx_delete();
	void on_fx_enabled();
	void on_fx_copy_from_track();
	void on_add_fx();
	void on_midi_fx_select();
	void on_midi_fx_edit();
	void on_midi_fx_move();
	void on_add_midi_fx();
	void update_fx_list_selection();
	void update();
};

