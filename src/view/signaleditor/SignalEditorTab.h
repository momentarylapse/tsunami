/*
 * SignalEditorTab.h
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#pragma once

#include "../../lib/hui/hui.h"
#include "../mainview/MainViewNode.h"

namespace tsunami {

class SignalChain;
class Session;
class AudioView;
class Module;
struct Cable;
enum class ModuleCategory;
enum class SignalType;
class SignalEditorModule;
class SignalEditorCable;
class SignalEditorBackground;
class ScrollPad;
class ScrollBar;

// TODO rename to SignalEditor
class SignalEditorTab : public MainViewNode {
public:
	SignalEditorTab(SignalChain *_chain);
	~SignalEditorTab();

	obs::xsource<Module*> out_module_selected{this, "module-selected"};
	obs::source out_popup_chain{this, "popup-chain"};
	obs::source out_popup_module{this, "popup-module"};

	Session *session;
	AudioView *view;
	SignalChain *chain;
	ScrollPad *pad;
	SignalEditorBackground *background;
	Array<SignalEditorModule*> modules;
	Array<SignalEditorCable*> cables;
	base::set<Module*> sel_modules;

	SignalEditorModule *get_module(Module *m);

	void select_module(Module *m, bool add=false);
	void update_module_positions();

	void* main_view_data() const override;
	string main_view_description() const override;
	void on_enter_main_view() override;

	static color signal_color_base(SignalType type);
	static color signal_color(SignalType type, bool hover);
	static void draw_arrow(Painter *p, const vec2 &m, const vec2 &_d, float length);

	bool on_key_down(int k) override;
	void on_chain_update();
	void on_activate();
	void on_delete();
	void on_add(ModuleCategory type);
	void on_reset();
	void on_save();
	void on_module_delete();


	void popup_chain();
	void popup_module();
};

}

