/*
 * SignalEditor.h
 *
 *  Created on: 30.03.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_SIGNALEDITOR_H_
#define SRC_VIEW_BOTTOMBAR_SIGNALEDITOR_H_


#include "BottomBar.h"

class Painter;
class SignalChain;
class Module;
namespace hui{
	class Menu;
}
class SignalEditorTab;
class ConfigPanel;

class SignalEditor: public BottomBar::Console
{
public:
	SignalEditor(Session *session);
	virtual ~SignalEditor();

	void addChain(SignalChain *c);

	void onNew();
	void onLoad();
	void deleteChain(SignalChain *c);

	Array<SignalEditorTab*> tabs;

	void on_chain_switch();

	string grid_id;
	string config_grid_id;
	Module *config_module;
	ConfigPanel *config_panel;

	void show_config(Module *m);

	hui::Menu *menu_chain, *menu_module;
};

#endif /* SRC_VIEW_BOTTOMBAR_SIGNALEDITOR_H_ */
