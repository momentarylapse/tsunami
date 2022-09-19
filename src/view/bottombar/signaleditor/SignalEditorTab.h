/*
 * SignalEditorTab.h
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#pragma once

#include "../../../lib/hui/hui.h"
#include "../../helper/graph/Node.h"

class SignalEditor;
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


class SignalEditorTab : public hui::Panel {
public:
	SignalEditorTab(SignalEditor *ed, SignalChain *_chain);
	virtual ~SignalEditorTab();

	SignalEditor *editor;
	Session *session;
	AudioView *view;
	SignalChain *chain;

	owned<scenegraph::SceneGraph> graph;
	ScrollPad *pad;
	SignalEditorBackground *background;
	Array<SignalEditorModule*> modules;
	Array<SignalEditorCable*> cables;
	base::set<Module*> sel_modules;

	SignalEditorModule *get_module(Module *m);

	void select_module(Module *m, bool add=false);
	void update_module_positions();


	color signal_color_base(SignalType type);
	color signal_color(SignalType type, bool hover);
	void draw_arrow(Painter *p, const vec2 &m, const vec2 &_d, float length);

	void on_draw(Painter* p);
	void on_chain_update();
	void on_chain_delete();
	void on_key_down();
	void on_activate();
	void on_delete();
	void on_add(ModuleCategory type);
	void on_reset();
	void on_save();
	void on_module_delete();
	void on_module_configure();


	void popup_chain();
	void popup_module();

	hui::Menu *menu_chain, *menu_module;
};

