/*
 * PluginConsole.h
 *
 *  Created on: 23.08.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_PLUGINCONSOLE_H_
#define SRC_VIEW_BOTTOMBAR_PLUGINCONSOLE_H_

#include "BottomBar.h"

//class PluginPanel;
//class ConfigPanel;
class ModulePanel;

class PluginConsole: public BottomBar::Console {
public:
	PluginConsole(Session *session, BottomBar *bar);
	~PluginConsole() override;

	void on_add_button();

	void on_add_plugin();
	void on_remove_plugin();

	void load_data();

	Array<ModulePanel*> panels;
	int next_x;
	//ConfigPanel *big_panel;
};

#endif /* SRC_VIEW_BOTTOMBAR_PLUGINCONSOLE_H_ */
