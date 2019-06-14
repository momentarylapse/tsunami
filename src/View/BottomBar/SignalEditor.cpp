/*
 * SignalEditor.cpp
 *
 *  Created on: 30.03.2018
 *      Author: michi
 */

#include "SignalEditor.h"

#include "../../Module/Port/Port.h"
#include "../AudioView.h"
#include "../../Session.h"
#include "../../Storage/Storage.h"
#include "../../Plugins/PluginManager.h"
#include "../../Module/Module.h"
#include "../../Module/ConfigPanel.h"
#include "../../lib/math/complex.h"
#include "../../Data/base.h"
#include "../../Module/SignalChain.h"
#include "../../TsunamiWindow.h"
#include "../../lib/math/interpolation.h"
//template class Interpolator<complex>;



const float MODULE_WIDTH = 160;
const float MODULE_HEIGHT = 25;
extern const int CONFIG_PANEL_WIDTH = 380;

static rect module_rect(Module *m) {
	return rect(m->module_x, m->module_x + MODULE_WIDTH, m->module_y, m->module_y + MODULE_HEIGHT);
}

static float module_port_in_x(Module *m) {
	return m->module_x - 5;
}

static float module_port_in_y(Module *m, int index) {
	return m->module_y + MODULE_HEIGHT/2 + (index - (float)(m->port_in.num-1)/2)*20;
}

static float module_port_out_x(Module *m) {
	return m->module_x + MODULE_WIDTH + 5;
}

static float module_port_out_y(Module *m, int index) {
	return m->module_y + MODULE_HEIGHT/2 + (index - (float)(m->port_out.num-1)/2)*20;
}

string module_header(Module *m) {
	if (m->module_subtype.num > 0)
		return m->module_subtype;
	return m->type_to_name(m->module_type);
}


class SignalEditorTab : public hui::Panel {
public:
	SignalEditorTab(SignalEditor *ed, SignalChain *_chain) {
		add_grid("", 0, 0, "grid");
		set_target("grid");
		add_drawing_area("!expandx,expandy,grabfocus", 0, 0, "area");

		editor = ed;
		view = ed->view;
		session = ed->session;

		event_xp("area", "hui:draw", [=](Painter *p){ on_draw(p); });
		event_x("area", "hui:mouse-move", [=]{ on_mouse_move(); });
		event_x("area", "hui:left-button-down", [=]{ on_left_button_down(); });
		event_x("area", "hui:left-button-up", [=]{ on_left_button_up(); });
		event_x("area", "hui:right-button-down", [=]{ on_right_button_down(); });
		event_x("area", "hui:key-down", [=]{ on_key_down(); });


		event("signal_chain_add_audio_source", [=]{ on_add(ModuleType::AUDIO_SOURCE); });
		event("signal_chain_add_audio_effect", [=]{ on_add(ModuleType::AUDIO_EFFECT); });
		event("signal_chain_add_stream", [=]{ on_add(ModuleType::STREAM); });
		event("signal_chain_add_plumbing", [=]{ on_add(ModuleType::PLUMBING); });
		event("signal_chain_add_audio_visualizer", [=]{ on_add(ModuleType::AUDIO_VISUALIZER); });
		event("signal_chain_add_midi_source", [=]{ on_add(ModuleType::MIDI_SOURCE); });
		event("signal_chain_add_midi_effect", [=]{ on_add(ModuleType::MIDI_EFFECT); });
		event("signal_chain_add_synthesizer", [=]{ on_add(ModuleType::SYNTHESIZER); });
		event("signal_chain_add_pitch_detector", [=]{ on_add(ModuleType::PITCH_DETECTOR); });
		event("signal_chain_add_beat_source", [=]{ on_add(ModuleType::BEAT_SOURCE); });
		event("signal_chain_reset", [=]{ on_reset(); });
		event("signal_chain_activate", [=]{ on_activate(); });
		event("signal_chain_delete", [=]{ on_delete(); });
		event("signal_chain_save", [=]{ on_save(); });
		event("signal_module_delete", [=]{ on_module_delete(); });
		event("signal_module_configure", [=]{ on_module_configure(); });

		event("signal_chain_new", [=]{ editor->on_new(); });
		event("signal_chain_load", [=]{ editor->on_load(); });

		chain = _chain;
		chain->subscribe(this, [=]{ on_chain_delete(); }, chain->MESSAGE_DELETE);
		chain->subscribe(this, [=]{ on_chain_update(); });
	}
	virtual ~SignalEditorTab() {
		chain->unsubscribe(this);
	}

	SignalEditor *editor;
	Session *session;
	AudioView *view;
	SignalChain *chain;
	rect area_play;

	struct Selection {
		Selection() {
			type = -1;
			module = nullptr;
			port = -1;
			port_type = SignalType::AUDIO;
			target_module = nullptr;
			target_port = -1;
			dx = dy = 0;
		}
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


	Selection get_hover(float mx, float my) {
		Selection s;
		s.dx = mx;
		s.dy = my;
		if (area_play.inside(mx, my)) {
			s.type = Selection::TYPE_BUTTON_PLAY;
			return s;
		}
		for (auto *m: chain->modules){
			rect r = module_rect(m);
			if (r.inside(mx, my)) {
				s.type = Selection::TYPE_MODULE;
				s.module = m;
				s.dx = m->module_x - mx;
				s.dy = m->module_y - my;
				return s;
			}
			for (int i=0; i<m->port_in.num; i++) {
				float y = module_port_in_y(m, i);
				float x = module_port_in_x(m);
				if (abs(x - mx) < 10 and abs(y - my) < 10) {
					s.type = Selection::TYPE_PORT_IN;
					s.module = m;
					s.port = i;
					s.port_type = m->port_in[i].type;
					s.dx = x;
					s.dy = y;
					return s;
				}
			}
			for (int i=0; i<m->port_out.num; i++) {
				float y = module_port_out_y(m, i);
				float x = module_port_out_x(m);
				if (abs(x - mx) < 10 and abs(y - my) < 10) {
					s.type = Selection::TYPE_PORT_OUT;
					s.module = m;
					s.port = i;
					s.port_type = m->port_out[i]->type;
					s.dx = x;
					s.dy = y;
					return s;
				}
			}
		}
		return s;
	}


	color signal_color(SignalType type, bool hover = false) {
		if (type == SignalType::AUDIO)
			return hover ? view->colors.red_hover : view->colors.red;
		if (type == SignalType::MIDI)
			return hover ? view->colors.green_hover : view->colors.green;
		if (type == SignalType::BEATS)
			return hover ? view->colors.blue_hover : view->colors.blue;
		return hover ? view->colors.white_hover : view->colors.white;
	}

	void draw_cable(Painter *p, SignalChain::Cable &c) {
		complex p0 = complex(module_port_out_x(c.source), module_port_out_y(c.source, c.source_port));
		complex p1 = complex(module_port_in_x(c.target), module_port_in_y(c.target, c.target_port));

		float length = (p1 - p0).abs();
		Interpolator<complex> inter(Interpolator<complex>::TYPE_CUBIC_SPLINE);
		inter.add2(p0, complex(length,0));
		inter.add2(p1, complex(length,0));

		p->set_color(signal_color(c.type));

		complex qq = complex::ZERO;
		for (float t=0; t<1.0f; t+=0.025f) {
			complex q = inter.get(t);
			if (t > 0)
				p->draw_line(qq.x, qq.y, q.x, q.y);
			qq = q;
		}
		complex m = inter.get(0.5f);
		complex d = inter.getTang(0.5);
		d /= d.abs();
		complex e = d * complex::I;
		Array<complex> pp;
		float arrow_length = min(length / 7, 18.0f);
		pp.add(m + d * arrow_length);
		pp.add(m - d * arrow_length + e * arrow_length / 2);
		pp.add(m - d * arrow_length - e * arrow_length / 2);
		p->draw_polygon(pp);
		//p->dr
	}

	void draw_module(Painter *p, Module *m) {
		p->set_color(view->colors.background_track_selected);
		if (hover.type == Selection::TYPE_MODULE and hover.module == m)
			p->set_color(ColorInterpolate(view->colors.background_track_selected, view->colors.hover, 0.25f));
		p->set_roundness(view->CORNER_RADIUS);
		p->draw_rect(module_rect(m));
		p->set_roundness(0);
		if (sel.type == sel.TYPE_MODULE and sel.module == m) {
			p->set_color(view->colors.text);
			p->set_font("", 12, true, false);
		} else {
			p->set_color(view->colors.text_soft1);
		}
		string type = module_header(m);
		float ww = p->get_str_width(type);
		p->draw_str(m->module_x + MODULE_WIDTH/2 - ww/2, m->module_y + 6, type);
		p->set_font("", 12, false, false);
	}

	void draw_ports(Painter *p, Module *m) {
		foreachi(auto &pd, m->port_in, i) {
			bool hovering = (hover.type == Selection::TYPE_PORT_IN and hover.module == m and hover.port == i);
			p->set_color(signal_color(pd.type, hovering));
			float r = hovering ? 6 : 4;
			p->draw_circle(module_port_in_x(m), module_port_in_y(m, i), r);
		}
		foreachi(auto *pd, m->port_out, i) {
			bool hovering = (hover.type == Selection::TYPE_PORT_OUT and hover.module == m and hover.port == i);
			p->set_color(signal_color(pd->type, hovering));
			float r = hovering ? 6 : 4;
			p->draw_circle(module_port_out_x(m), module_port_out_y(m, i), r);
		}
	}

	void on_draw(Painter* p) {
		int w = p->width;
		int h = p->height;
		p->set_color(view->colors.background);
		p->draw_rect(0, 0, w, h);
		p->set_font_size(12);

		for (auto *m: chain->modules)
			draw_module(p, m);

		for (auto &c: chain->cables())
			draw_cable(p, c);

		for (auto *m: chain->modules)
			draw_ports(p, m);

		for (auto &pp: chain->_ports_out){
			p->set_color(Red);
			p->draw_circle(module_port_out_x(pp.module)+20, module_port_out_y(pp.module, pp.port), 10);
		}

		if (sel.type == sel.TYPE_PORT_IN or sel.type == sel.TYPE_PORT_OUT) {
			p->set_color(White);
			if (hover.target_module) {
				p->set_line_width(5);
				Module *t = hover.target_module;
				if (hover.type == hover.TYPE_PORT_IN)
					p->draw_line(sel.dx, sel.dy, module_port_out_x(t), module_port_out_y(t, hover.target_port));
				else
					p->draw_line(sel.dx, sel.dy, module_port_in_x(t), module_port_in_y(t, hover.target_port));
				p->set_line_width(1);
			} else {
				p->draw_line(sel.dx, sel.dy, hui::GetEvent()->mx, hui::GetEvent()->my);
			}
		}


		float mx = hui::GetEvent()->mx;
		float my = hui::GetEvent()->my;
		Selection hh = get_hover(mx, my);
		if (hh.type == hover.TYPE_PORT_IN)
			AudioView::draw_cursor_hover(p, _("input: ") + signal_type_name(hh.port_type), mx, my, p->area());
		if (hh.type == hover.TYPE_PORT_OUT)
			AudioView::draw_cursor_hover(p, _("output: ") + signal_type_name(hh.port_type), mx, my, p->area());

		area_play = rect(10, 30, p->height - 30, p->height - 10);
		p->set_color(view->colors.text);
		if (hover.type == hover.TYPE_BUTTON_PLAY)
			p->set_color(view->colors.hover);
		p->draw_str(area_play.x1, area_play.y1, chain->is_playback_active() ? "⏹" : "▶️");
	}

	void on_chain_update() {
		redraw("area");
	}

	void on_chain_delete() {
		editor->remove_tab(this);
	}

	void on_left_button_down() {
		float mx = hui::GetEvent()->mx;
		float my = hui::GetEvent()->my;
		hover = get_hover(mx, my);
		sel = hover;
		apply_sel();
		if (sel.type == sel.TYPE_PORT_IN) {
			chain->disconnect_in(sel.module, sel.port);
		} else if (sel.type == sel.TYPE_PORT_OUT) {
			chain->disconnect_out(sel.module, sel.port);
		} else if (sel.type == sel.TYPE_BUTTON_PLAY) {
			if (chain->is_playback_active())
				chain->stop();
			else
				chain->start();
		}
		redraw("area");
	}

	void on_left_button_up() {
		if (sel.type == sel.TYPE_PORT_IN or sel.type == sel.TYPE_PORT_OUT) {
			if (hover.target_module) {
				if (sel.type == sel.TYPE_PORT_IN) {
					chain->disconnect_out(hover.target_module, hover.target_port);
					chain->connect(hover.target_module, hover.target_port, sel.module, sel.port);
				} else if (sel.type == sel.TYPE_PORT_OUT) {
					chain->disconnect_in(hover.target_module, hover.target_port);
					chain->connect(sel.module, sel.port, hover.target_module, hover.target_port);
				}
			}
			sel = Selection();
			apply_sel();
		}
		redraw("area");
	}

	void on_mouse_move() {
		float mx = hui::GetEvent()->mx;
		float my = hui::GetEvent()->my;
		if (hui::GetEvent()->lbut) {
			if (sel.type == sel.TYPE_MODULE) {
				auto *m = sel.module;
				m->module_x = mx + sel.dx;
				m->module_y = my + sel.dy;
			} else if (sel.type == sel.TYPE_PORT_IN or sel.type == sel.TYPE_PORT_OUT) {
				hover.target_module = nullptr;
				auto h = get_hover(mx, my);
				if (h.module != sel.module and h.port_type == sel.port_type) {
					if (h.type == sel.TYPE_PORT_IN and sel.type == sel.TYPE_PORT_OUT) {
						hover.target_module = h.module;
						hover.target_port = h.port;
					}
					if (h.type == sel.TYPE_PORT_OUT and sel.type == sel.TYPE_PORT_IN) {
						hover.target_module = h.module;
						hover.target_port = h.port;
					}
				}
			}
		} else {
			hover = get_hover(mx, my);
		}
		redraw("area");
	}

	void on_right_button_down() {
		float mx = hui::GetEvent()->mx;
		float my = hui::GetEvent()->my;
		hover = get_hover(mx, my);
		sel = hover;
		apply_sel();

		if (hover.type == hover.TYPE_MODULE) {
			editor->menu_module->open_popup(this);
		} else if (hover.type < 0) {
			editor->menu_chain->open_popup(this);
		}
	}

	void on_key_down() {
		int key = hui::GetEvent()->key_code;
		if (key == hui::KEY_DELETE) {
			if (sel.type == sel.TYPE_MODULE) {
				chain->delete_module(sel.module);
				hover = sel = Selection();
			}
		}
	}

	void apply_sel() {
		editor->show_config(sel.module);
	}

	void on_activate() {
	}

	void on_delete() {
		if (chain->belongs_to_system) {
			session->e(_("not allowed to delete the main signal chain"));
			return;
		}
		hui::RunLater(0.001f, [=](){ delete chain; });
	}


	void on_add(ModuleType type) {
		Array<string> names = session->plugin_manager->find_module_sub_types(type);
		if (names.num > 1) {
			string name = session->plugin_manager->choose_module(win, session, type);
			if (name.num > 0) {
				auto *m = chain->add(type, name);
				m->module_x = sel.dx;
				m->module_y = sel.dy;
			}
		} else {
			auto *m = chain->add(type);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void on_reset() {
		chain->reset();
	}

	/*void on_load() {
		if (hui::FileDialogOpen(win, "", "", "*.chain", "*.chain"))
			chain->load(hui::Filename);
	}*/

	void on_save() {
		if (hui::FileDialogSave(win, _("Save the signal chain"), session->storage->current_chain_directory, "*.chain", "*.chain")) {
			session->storage->current_chain_directory = hui::Filename.dirname();
			chain->save(hui::Filename);
		}
	}



	void on_module_delete() {
		if (sel.type == sel.TYPE_MODULE) {
			chain->delete_module(sel.module);
			hover = sel = Selection();
			apply_sel();
		}
	}

	void on_module_configure() {
		apply_sel();
	}

};

SignalEditor::SignalEditor(Session *session) :
	BottomBar::Console(_("Signal Chain"), session)
{
	grid_id = "main-grid";
	config_grid_id = "config-panel-grid";
	add_grid("", 0, 0, grid_id);
	set_target(grid_id);
	add_tab_control("!left\\aaa", 0, 0, "selector");
	add_revealer("!slide=left", 1, 0, "revealer");
	set_target("revealer");
	add_grid(format("!width=%d,noexpandx", CONFIG_PANEL_WIDTH), 1, 0, config_grid_id);
	set_target(config_grid_id);
	add_label("!bold,center,big,expandx", 0, 0, "config-label");
	add_label("!bold,center,expandx", 0, 1, "message");

	menu_chain = hui::CreateResourceMenu("popup_signal_chain_menu");
	menu_module = hui::CreateResourceMenu("popup_signal_module_menu");

	config_module = nullptr;
	config_panel = nullptr;

	event("selector", [=]{ on_chain_switch(); });

	for (auto *c: session->all_signal_chains)
		add_chain(c);
	show_config(nullptr);

	session->subscribe(this, [=] { add_chain(session->all_signal_chains.back()); }, session->MESSAGE_ADD_SIGNAL_CHAIN);
}

SignalEditor::~SignalEditor() {
	session->unsubscribe(this);
	show_config(nullptr);
	delete menu_chain;
	delete menu_module;
}

void SignalEditor::add_chain(SignalChain *c) {
	auto *tab = new SignalEditorTab(this, c);
	int index = tabs.num;
	string grid_id = "grid-" + i2s(index);
	if (index > 0)
		add_string("selector", c->name);
	else
		change_string("selector", index, c->name);
	set_target("selector");
	add_grid("", index, 0, grid_id);
	embed(tab, grid_id, 0, 0);
	tabs.add(tab);

	set_int("selector", index);
}

void SignalEditor::on_new() {
	session->add_signal_chain("new");
	//add_chain();
}

void SignalEditor::on_load() {
	if (hui::FileDialogOpen(win, _("Load a signal chain"), session->storage->current_chain_directory, "*.chain", "*.chain")) {
		session->storage->current_chain_directory = hui::Filename.dirname();
		auto *c = SignalChain::load(session, hui::Filename);
		add_chain(c);
	}
}

void SignalEditor::on_chain_switch() {
	//msg_write("switch");
}

void SignalEditor::show_config(Module *m) {
	if (m and (m == config_module))
		return;
	if (config_panel)
		delete config_panel;
	config_panel = nullptr;
	config_module = m;
	if (m) {
		set_string("config-label", module_header(m));
		config_panel = m->create_panel();
		if (config_panel) {
			config_panel->update();
			embed(config_panel, config_grid_id, 0, 2);
			config_panel->set_large(false);
			//setOptions(config_grid_id, "width=330,noexpandx");
			hide_control("message", true);
		} else {
			set_string("message", _("module not configurable"));
			hide_control("message", false);
		}
	} else {
		set_string("config-label", "");
		set_string("message", _("no module selected"));
		hide_control("message", false);
	}
	reveal("revealer", m);
}

void SignalEditor::remove_tab(SignalEditorTab *t) {
	foreachi(auto *tt, tabs, i)
		if (tt == t) {
			delete t;
			tabs.erase(i);
			remove_string("selector", i);
			set_int("selector", 0);
		}
}
