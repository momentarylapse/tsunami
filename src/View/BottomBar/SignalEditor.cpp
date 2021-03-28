/*
 * SignalEditor.cpp
 *
 *  Created on: 30.03.2018
 *      Author: michi
 */

#include "SignalEditor.h"
#include "../Helper/ModulePanel.h"
#include "../AudioView.h"

#include "../../Module/Port/Port.h"
#include "../../Module/Module.h"
#include "../../Module/ConfigPanel.h"
#include "../../Module/SignalChain.h"
#include "../../Session.h"
#include "../../Storage/Storage.h"
#include "../../Plugins/PluginManager.h"
#include "../../lib/math/complex.h"
#include "../../Data/base.h"
#include "../../TsunamiWindow.h"
#include "../../lib/math/interpolation.h"
//template class Interpolator<complex>;



const float MODULE_WIDTH = 140;
const float MODULE_HEIGHT = 23;
const float MODULE_GRID = 23;

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
	if (m->module_name.num > 0)
		return m->module_name;
	if (m->module_class.num > 0)
		return m->module_class;
	return m->category_to_str(m->module_category);
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
		event_x("area", "hui:mouse-wheel", [=]{ on_mouse_wheel(); });
		event_x("area", "hui:key-down", [=]{ on_key_down(); });


		event("signal_chain_add_audio_source", [=]{ on_add(ModuleCategory::AUDIO_SOURCE); });
		event("signal_chain_add_audio_effect", [=]{ on_add(ModuleCategory::AUDIO_EFFECT); });
		event("signal_chain_add_stream", [=]{ on_add(ModuleCategory::STREAM); });
		event("signal_chain_add_plumbing", [=]{ on_add(ModuleCategory::PLUMBING); });
		event("signal_chain_add_audio_visualizer", [=]{ on_add(ModuleCategory::AUDIO_VISUALIZER); });
		event("signal_chain_add_midi_source", [=]{ on_add(ModuleCategory::MIDI_SOURCE); });
		event("signal_chain_add_midi_effect", [=]{ on_add(ModuleCategory::MIDI_EFFECT); });
		event("signal_chain_add_synthesizer", [=]{ on_add(ModuleCategory::SYNTHESIZER); });
		event("signal_chain_add_pitch_detector", [=]{ on_add(ModuleCategory::PITCH_DETECTOR); });
		event("signal_chain_add_beat_source", [=]{ on_add(ModuleCategory::BEAT_SOURCE); });
		event("signal_chain_reset", [=]{ on_reset(); });
		event("signal_chain_activate", [=]{ on_activate(); });
		event("signal_chain_delete", [=]{ on_delete(); });
		event("signal_chain_save", [=]{ on_save(); });
		event("signal_module_delete", [=]{ on_module_delete(); });
		event("signal_module_configure", [=]{ on_module_configure(); });

		event("signal_chain_new", [=]{ editor->on_new(); });
		event("signal_chain_load", [=]{ editor->on_load(); });

		chain = _chain;
		chain->subscribe(this, [=]{ on_chain_update(); }, chain->MESSAGE_STATE_CHANGE);
		//chain->subscribe(this, [=]{ on_chain_update(); }, chain->MESSAGE_PLAY_END_OF_STREAM);
		chain->subscribe(this, [=]{ on_chain_update(); }, chain->MESSAGE_DELETE_CABLE);
		chain->subscribe(this, [=]{ on_chain_update(); }, chain->MESSAGE_DELETE_MODULE);
		chain->subscribe(this, [=]{ on_chain_update(); }, chain->MESSAGE_ADD_CABLE);
		chain->subscribe(this, [=]{ on_chain_update(); }, chain->MESSAGE_ADD_MODULE);
		chain->subscribe(this, [=]{ on_chain_delete(); }, chain->MESSAGE_DELETE);
	}
	virtual ~SignalEditorTab() {
		chain->unsubscribe(this);
	}

	SignalEditor *editor;
	Session *session;
	AudioView *view;
	SignalChain *chain;
	rect area_play;

	complex view_offset = complex(0,0);
	float view_zoom = 1;

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
		mx -= view_offset.x;
		my -= view_offset.y;
		for (auto *m: weak(chain->modules)){
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


	color signal_color_base(SignalType type) {
		if (type == SignalType::AUDIO)
			return view->colors.red;
		if (type == SignalType::MIDI)
			return view->colors.green;
		if (type == SignalType::BEATS)
			return view->colors.blue;
		return view->colors.white;
	}

	color signal_color(SignalType type, bool hover) {
		color c = signal_color_base(type);
		c = color::interpolate(c, view->colors.text, 0.2f);
		//c = color::interpolate(c, view->colors.background, 0.2f);
		if (hover)
			c = view->colors.hoverify(c);
		return c;
	}
	
	void draw_arrow(Painter *p, const complex &m, const complex &_d, float length) {
		complex d = _d / _d.abs();
		complex e = d * complex::I;
		Array<complex> pp;
		pp.add(m + d * length);
		pp.add(m - d * length + e * length / 2);
		pp.add(m - d * length * 0.8f);
		pp.add(m - d * length - e * length / 2);
		p->draw_polygon(pp);
	}

	void draw_cable(Painter *p, SignalChain::Cable &c) {
		complex p0 = complex(module_port_out_x(c.source), module_port_out_y(c.source, c.source_port));
		complex p1 = complex(module_port_in_x(c.target), module_port_in_y(c.target, c.target_port));

		float length = (p1 - p0).abs();
		Interpolator<complex> inter(Interpolator<complex>::TYPE_CUBIC_SPLINE);
		inter.add2(p0, complex(length,0));
		inter.add2(p1, complex(length,0));

		color base_color = signal_color(c.type, false);

		// curve
		p->set_color(base_color);
		//p->set_color(color::interpolate(base_color, view->colors.background, 0.1f));
		p->set_line_width(2.0f);
		p->set_line_dash({5, 2}, 0);
		Array<complex> cc;
		for (float t=0; t<=1.0f; t+=0.025f)
			cc.add(inter.get(t));
		p->draw_lines(cc);
		p->set_line_dash({}, 0);
		p->set_line_width(1);

		p->set_color(color::interpolate(base_color, view->colors.text, 0.1f));
		//p->set_color(base_color);
		draw_arrow(p, inter.get(0.5f), inter.getTang(0.5f), min(length / 7, 14.0f));
	}

	void draw_module(Painter *p, Module *m) {
		color bg = view->colors.blob_bg;
		if (sel.type == sel.TYPE_MODULE and sel.module == m)
			bg = view->colors.blob_bg_selected;
		if (hover.type == Selection::TYPE_MODULE and hover.module == m)
			bg = view->colors.hoverify(bg);
		p->set_color(bg);
		p->set_roundness(view->CORNER_RADIUS);
		rect r = module_rect(m);
		p->draw_rect(r);
		p->set_roundness(0);
		p->set_font_size(AudioView::FONT_SIZE);// * 1.2f);
		if (sel.type == sel.TYPE_MODULE and sel.module == m) {
			p->set_color(view->colors.text);
			p->set_font("", -1, true, false);
		} else {
			p->set_color(view->colors.text_soft1);
		}
		string type = module_header(m);
		AudioView::draw_str_constrained(p, r.mx(), r.my() - p->font_size/2, r.width() - 12, type, 0);
		p->set_font("", AudioView::FONT_SIZE, false, false);
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

	void draw_background(Painter *p) {
		int w = p->width;
		int h = p->height;
		p->set_color(view->colors.background);
		p->draw_rect(0, 0, w, h);
		p->set_line_width(0.7f);


		float rot[4] = {1,0,0,1};
		p->set_transform(rot, view_offset);

		float D = MODULE_GRID;
		int i0 = -view_offset.x / D;
		int i1 = (w - view_offset.x) / D;
		for (int i=i0; i<=i1; i++) {
			float x = i * D;
			if ((i % 5) == 0)
				p->set_color(color::interpolate(view->colors.background, view->colors.grid, 0.5f));
			else
				p->set_color(color::interpolate(view->colors.background, view->colors.grid, 0.2f));
			p->draw_line(x, -view_offset.y, x, h - view_offset.y);
		}
		int j0 = -view_offset.y / D;
		int j1 = (h - view_offset.y) / D;
		for (int j=j0; j<=j1; j++) {
			float y = j * D;
			if ((j % 5) == 0)
				p->set_color(color::interpolate(view->colors.background, view->colors.grid, 0.5f));
			else
				p->set_color(color::interpolate(view->colors.background, view->colors.grid, 0.2f));
			p->draw_line(-view_offset.x, y, w - view_offset.x, y);
		}

		p->set_line_width(1);
	}

	void on_draw(Painter* p) {
		int w = p->width;
		int h = p->height;
		draw_background(p);
		p->set_font_size(12);

		for (auto *m: weak(chain->modules))
			draw_module(p, m);

		for (auto &c: chain->cables())
			draw_cable(p, c);

		for (auto *m: weak(chain->modules))
			draw_ports(p, m);

		for (auto &pp: chain->_ports_out){
			p->set_color(Red);
			p->draw_circle(module_port_out_x(pp.module)+20, module_port_out_y(pp.module, pp.port), 10);
		}

		if (sel.type == sel.TYPE_PORT_IN or sel.type == sel.TYPE_PORT_OUT) {
			p->set_color(view->colors.text);
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


		float rot0[] = {1,0,0,1};
		p->set_transform(rot0, complex::ZERO);
		area_play = rect(10, 30, p->height - 30, p->height - 10);
		p->set_color(view->colors.text);
		if (hover.type == hover.TYPE_BUTTON_PLAY)
			p->set_color(view->colors.hover);
		p->draw_str(area_play.x1, area_play.y1, chain->is_playback_active() ? u8"\u23F9" : u8"\u25B6");
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
		} else if (sel.type == sel.TYPE_MODULE) {
			auto *m = sel.module;
			m->module_x = MODULE_GRID * (int)(m->module_x / MODULE_GRID + 0.5f);
			m->module_y = MODULE_GRID * (int)(m->module_y / MODULE_GRID + 0.5f);
		}
		redraw("area");
	}

	void on_mouse_move() {
		float mx = hui::GetEvent()->mx;
		float my = hui::GetEvent()->my;
		if (hui::GetEvent()->lbut) {
			if (sel.type == sel.TYPE_MODULE) {
				auto *m = sel.module;
				m->module_x = mx + sel.dx - view_offset.x;
				m->module_y = my + sel.dy - view_offset.y;
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
		if (key == hui::KEY_UP)
			view_offset.y += 10;
		if (key == hui::KEY_DOWN)
			view_offset.y -= 10;
		if (key == hui::KEY_LEFT)
			view_offset.x += 10;
		if (key == hui::KEY_RIGHT)
			view_offset.x -= 10;
		redraw("area");
	}

	void on_mouse_wheel() {
		float dx = hui::GetEvent()->scroll_x;
		float dy = hui::GetEvent()->scroll_y;
		view_offset -= complex(dx, dy) * 10;
		// TODO restrict...
		redraw("area");
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
		hui::RunLater(0.001f, [=](){ chain->unregister(); });
	}


	void on_add(ModuleCategory type) {
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
		chain->reset(false);
	}

	/*void on_load() {
		if (hui::FileDialogOpen(win, "", "", "*.chain", "*.chain"))
			chain->load(hui::Filename);
	}*/

	void on_save() {
		if (hui::FileDialogSave(win, _("Save the signal chain"), session->storage->current_chain_directory, "*.chain", "*.chain")) {
			session->storage->current_chain_directory = hui::Filename.parent();
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
	add_grid("!noexpandx", 1, 0, config_grid_id);
	set_target(config_grid_id);
	//add_label("!bold,center,big,expandx", 0, 0, "config-label");
	//add_label("!bold,center,expandx", 0, 1, "message");

	menu_chain = hui::CreateResourceMenu("popup_signal_chain_menu");
	menu_module = hui::CreateResourceMenu("popup_signal_module_menu");

	config_module = nullptr;
	config_panel = nullptr;

	event("selector", [=]{ on_chain_switch(); });

	for (auto *c: weak(session->all_signal_chains))
		add_chain(c);
	show_config(nullptr);

	session->subscribe(this, [=] { add_chain(session->all_signal_chains.back().get()); }, session->MESSAGE_ADD_SIGNAL_CHAIN);
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
	session->create_signal_chain("new");
}

void SignalEditor::on_load() {
	if (hui::FileDialogOpen(win, _("Load a signal chain"), session->storage->current_chain_directory, "*.chain", "*.chain")) {
		session->storage->current_chain_directory = hui::Filename.parent();
		session->load_signal_chain(hui::Filename);
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
		config_panel = new ModulePanel(config_module);
		config_panel->set_func_delete([=] {
			for (auto *chain: weak(session->all_signal_chains))
				for (auto *_m: weak(chain->modules))
					if (m == _m)
						chain->delete_module(m);
		});
		embed(config_panel, config_grid_id, 0, 0);
	} else {
		/*set_string("config-label", "");
		set_string("message", _("no module selected"));
		hide_control("message", false);*/
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
