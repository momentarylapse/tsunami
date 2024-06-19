/*
 * PluginConsole.h
 *
 *  Created on: 23.08.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_PLUGINCONSOLE_H_
#define SRC_VIEW_BOTTOMBAR_PLUGINCONSOLE_H_

#include "BottomBar.h"

namespace tsunami {

//class PluginPanel;
//class ConfigPanel;
class ModulePanel;
class TsunamiPlugin;

class PluginConsole: public BottomBar::Console {
public:
	PluginConsole(Session *session, BottomBar *bar);
	~PluginConsole() override;

	void on_enter() override;

	void on_add_button();

	void on_add_plugin(TsunamiPlugin *p);
	void on_remove_plugin(TsunamiPlugin *p);

	void update_favotites();

	shared_array<ModulePanel> panels;
	int next_x;

	struct FavoriteButtonData {
		string id;
		int handler;
	};
	Array<FavoriteButtonData> favorite_buttons;
};

}

#endif /* SRC_VIEW_BOTTOMBAR_PLUGINCONSOLE_H_ */
