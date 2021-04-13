/*
 * SignalEditorTab.h
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#pragma once

#include "../../../lib/hui/hui.h"
#include "../../Helper/Graph/Node.h"

class SignalEditor;
class SignalChain;
class Session;
class AudioView;
class ScrollBar;
class Module;
struct Cable;
enum class ModuleCategory;
enum class SignalType;


class SignalEditorTab : public hui::Panel {
public:
	SignalEditorTab(SignalEditor *ed, SignalChain *_chain);
	virtual ~SignalEditorTab();

	SignalEditor *editor;
	Session *session;
	AudioView *view;
	SignalChain *chain;

	complex view_offset = complex(0,0);
	float view_zoom = 1;

	struct Selection {
		Selection();
		int type;
		Module *module;
		int port;
		SignalType port_type;
		float dx, dy;
		enum {
			TYPE_MODULE,
			TYPE_PORT_IN,
			TYPE_PORT_OUT,
			TYPE_BUTTON_PLAY,
		};
		Module *target_module;
		int target_port;
	};
	//Selection getHover(float mx, float my);
	Selection hover, sel;

	owned<scenegraph::SceneGraph> graph;
	ScrollBar *scroll_bar_h;
	ScrollBar *scroll_bar_v;


	Selection get_hover(float mx, float my);
	color signal_color_base(SignalType type);
	color signal_color(SignalType type, bool hover);
	void draw_arrow(Painter *p, const complex &m, const complex &_d, float length);
	void draw_cable(Painter *p, Cable &c);
	void draw_module(Painter *p, Module *m);
	void draw_ports(Painter *p, Module *m);
	void on_draw(Painter* p);
	void on_chain_update();
	void on_chain_delete();
	void on_left_button_down();
	void on_left_button_up();
	void on_mouse_move();
	void on_right_button_down();
	void on_key_down();
	void move_cam(float dx, float dy);
	void on_mouse_wheel();
	void apply_sel();
	void on_activate();
	void on_delete();
	void on_add(ModuleCategory type);
	void on_reset();
	void on_save();
	void on_module_delete();
	void on_module_configure();



};

