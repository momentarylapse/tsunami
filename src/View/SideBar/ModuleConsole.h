/*
 * ModuleConsole.h
 *
 *  Created on: 03.04.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_MODULECONSOLE_H_
#define SRC_VIEW_SIDEBAR_MODULECONSOLE_H_


#include "SideBar.h"

class Module;
class ConfigPanel;

class ModuleConsole : public SideBarConsole
{
public:
	ModuleConsole(Session *session);
	virtual ~ModuleConsole();

	void clear();
	void setModule(Module *m);

	void onEditSong();

	void onModuleDelete();
	void onModuleSelectionChange();

	string id_inner;

	hui::Panel *panel;

	Module *module;
	ConfigPanel *module_panel;
};

#endif /* SRC_VIEW_SIDEBAR_MODULECONSOLE_H_ */
