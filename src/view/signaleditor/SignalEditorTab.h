/*
 * SignalEditorTab.h
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#pragma once

#include "../../lib/base/base.h"
#include "../../lib/base/set.h"
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
	explicit SignalEditorTab(SignalChain *_chain);
	~SignalEditorTab() override;

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

	void* mvn_data() const override;
	string mvn_description() const override;
	void mvn_on_enter() override;

	void play() override;
	void stop() override;
	void pause() override;
	bool is_playback_active() override;
	bool is_paused() override;
	bool mvn_can_play() override;
	bool mvn_can_pause() override;
	bool mvn_can_stop() override;

	static color signal_color_base(SignalType type);
	static color signal_color(SignalType type, bool hover);

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

