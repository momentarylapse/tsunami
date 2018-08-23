/*
 * PluginConsole.h
 *
 *  Created on: 23.08.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_PLUGINCONSOLE_H_
#define SRC_VIEW_BOTTOMBAR_PLUGINCONSOLE_H_

#include "BottomBar.h"

class PluginPanel;

class PluginConsole: public BottomBar::Console
{
public:
	PluginConsole(Session *session);
	virtual ~PluginConsole();

	void on_add_button();

	void on_add_plugin();
	void on_remove_plugin();

	void load_data();

	Array<PluginPanel*> panels;
	int next_x;
};

#endif /* SRC_VIEW_BOTTOMBAR_PLUGINCONSOLE_H_ */
