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
namespace hui {
	class Menu;
}
class SignalEditorTab;
class ModulePanel;

class SignalEditor: public BottomBar::Console {
public:
	SignalEditor(Session *session);
	virtual ~SignalEditor();

	void add_chain(SignalChain *c);

	void on_new();
	void on_load();
	void remove_tab(SignalEditorTab *t);

	Array<SignalEditorTab*> tabs;

	void on_chain_switch();

	string grid_id;
	string config_grid_id;
	Module *config_module;
	ModulePanel *config_panel;

	void show_config(Module *m);

	hui::Menu *menu_chain, *menu_module;
};

#endif /* SRC_VIEW_BOTTOMBAR_SIGNALEDITOR_H_ */
