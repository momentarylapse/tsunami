//
// Created by michi on 5/9/24.
//

#ifndef TSUNAMI_SIGNALCHAINPANEL_H
#define TSUNAMI_SIGNALCHAINPANEL_H

#include "SideBar.h"

class Session;
enum class ModuleCategory;
class SignalChain;
class SignalEditorTab;
class Module;
class ModulePanel;

class SignalChainPanel : public SideBarConsole {
public:
	SignalChainPanel(Session *session, SideBar *bar);
	~SignalChainPanel();

	obs::xsink<Module*> in_editor_module_selected;

	void on_search();
	void on_list();

	void on_delete();
	void on_save_as();

	void fill_module_list(const string& search);

//	void on_enter() override;
//	void on_leave() override;

	string id_list;

	struct ModuleDescriptor {
		ModuleCategory category;
		string name;
	};

	Array<ModuleDescriptor> modules;

	SignalEditorTab* editor = nullptr;
	SignalChain* chain();

	void set_module(Module* m);

	ModulePanel *module_panel = nullptr;
};


#endif //TSUNAMI_SIGNALCHAINPANEL_H
