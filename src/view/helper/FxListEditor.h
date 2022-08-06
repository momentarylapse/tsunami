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
enum class ConfigPanelMode;
namespace hui {
	class Panel;
	class Menu;
}


//todo set/unset track...
class FxListEditor : public VirtualBase {
public:
	FxListEditor(Track *t, hui::Panel *p, const string &_id, bool hexpand);
	~FxListEditor();
	hui::Panel *panel;
	string id_list;
	Track *track;
	Module *selected_module = nullptr;
	shared<ModulePanel> config_panel;
	Array<int> event_ids;
	ConfigPanelMode module_panel_mode;

	Session *session() const;
	void select_module(Module *m);
	void on_select();
	void on_edit();
	void on_move();
	owned<hui::Menu> menu;
	void on_right_click();
	void on_delete();
	void on_enabled();
	void on_copy_from_track();
	void on_add();
	void update_list_selection();
	void update();
};

