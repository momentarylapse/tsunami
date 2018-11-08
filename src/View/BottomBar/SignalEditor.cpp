/*
 * SignalEditor.cpp
 *
 *  Created on: 30.03.2018
 *      Author: michi
 */

#include "SignalEditor.h"

#include "../../Module/Port/MidiPort.h"
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
#include "../SideBar/SideBar.h"
#include "../SideBar/ModuleConsole.h"
#include "../../lib/math/interpolation.h"
//template class Interpolator<complex>;



const float MODULE_WIDTH = 160;
const float MODULE_HEIGHT = 25;

static rect module_rect(Module *m)
{
	return rect(m->module_x, m->module_x + MODULE_WIDTH, m->module_y, m->module_y + MODULE_HEIGHT);
}

static float module_port_in_x(Module *m)
{
	return m->module_x - 5;
}

static float module_port_in_y(Module *m, int index)
{
	return m->module_y + MODULE_HEIGHT/2 + (index - (float)(m->port_in.num-1)/2)*20;
}

static float module_port_out_x(Module *m)
{
	return m->module_x + MODULE_WIDTH + 5;
}

static float module_port_out_y(Module *m, int index)
{
	return m->module_y + MODULE_HEIGHT/2 + (index - (float)(m->port_out.num-1)/2)*20;
}

static string module_header(Module *m)
{
	if (m->module_subtype.num > 0)
		return m->module_subtype;
	return m->type_to_name(m->module_type);
}


class SignalEditorTab : public hui::Panel
{
public:
	SignalEditorTab(SignalEditor *ed, SignalChain *_chain)
	{
		addGrid("", 0, 0, "grid");
		setTarget("grid");
		addDrawingArea("!expandx,expandy,grabfocus", 0, 0, "area");

		eventXP("area", "hui:draw", std::bind(&SignalEditorTab::on_draw, this, std::placeholders::_1));
		eventX("area", "hui:mouse-move", std::bind(&SignalEditorTab::on_mouse_move, this));
		eventX("area", "hui:left-button-down", std::bind(&SignalEditorTab::on_left_button_down, this));
		eventX("area", "hui:left-button-up", std::bind(&SignalEditorTab::on_left_button_up, this));
		eventX("area", "hui:right-button-down", std::bind(&SignalEditorTab::on_right_button_down, this));
		eventX("area", "hui:key-down", std::bind(&SignalEditorTab::on_key_down, this));


		event("signal_chain_add_audio_source", std::bind(&SignalEditorTab::on_add_audio_source, this));
		event("signal_chain_add_audio_effect", std::bind(&SignalEditorTab::on_add_audio_effect, this));
		event("signal_chain_add_audio_input", std::bind(&SignalEditorTab::on_add_audio_input_stream, this));
		event("signal_chain_add_audio_joiner", std::bind(&SignalEditorTab::on_add_audio_joiner, this));
		event("signal_chain_add_audio_sucker", std::bind(&SignalEditorTab::on_add_audio_sucker, this));
		event("signal_chain_add_audio_visualizer", std::bind(&SignalEditorTab::on_add_audio_visualizer, this));
		event("signal_chain_add_midi_source", std::bind(&SignalEditorTab::on_add_midi_source, this));
		event("signal_chain_add_midi_effect", std::bind(&SignalEditorTab::on_add_midi_effect, this));
		event("signal_chain_add_midi_input", std::bind(&SignalEditorTab::on_add_midi_input_stream, this));
		event("signal_chain_add_synthesizer", std::bind(&SignalEditorTab::on_add_synthesizer, this));
		event("signal_chain_add_pitch_detector", std::bind(&SignalEditorTab::on_add_pitch_detector, this));
		event("signal_chain_add_beat_source", std::bind(&SignalEditorTab::on_add_beat_source, this));
		event("signal_chain_add_beat_midifier", std::bind(&SignalEditorTab::on_add_beat_midifier, this));
		event("signal_chain_reset", std::bind(&SignalEditorTab::on_reset, this));
		event("signal_chain_activate", std::bind(&SignalEditorTab::on_activate, this));
		event("signal_chain_delete", std::bind(&SignalEditorTab::on_delete, this));
		event("signal_chain_save", std::bind(&SignalEditorTab::on_save, this));
		event("signal_module_delete", std::bind(&SignalEditorTab::on_module_delete, this));
		event("signal_module_configure", std::bind(&SignalEditorTab::on_module_configure, this));

		event("signal_chain_new", std::bind(&SignalEditor::on_new, ed));
		event("signal_chain_load", std::bind(&SignalEditor::on_load, ed));

		editor = ed;
		view = ed->view;
		session = ed->session;

		chain = _chain;
		chain->subscribe(this, std::bind(&SignalEditorTab::on_chain_update, this));
	}
	virtual ~SignalEditorTab()
	{
		chain->unsubscribe(this);
	}

	SignalEditor *editor;
	Session *session;
	AudioView *view;
	SignalChain *chain;

	struct Selection
	{
		Selection()
		{
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
		enum{
			TYPE_MODULE,
			TYPE_PORT_IN,
			TYPE_PORT_OUT,
		};
		Module *target_module;
		int target_port;
	};
	//Selection getHover(float mx, float my);
	Selection hover, sel;


	Selection get_hover(float mx, float my)
	{
		Selection s;
		s.dx = mx;
		s.dy = my;
		for (auto *m: chain->modules){
			rect r = module_rect(m);
			if (r.inside(mx, my)){
				s.type = Selection::TYPE_MODULE;
				s.module = m;
				s.dx = m->module_x - mx;
				s.dy = m->module_y - my;
				return s;
			}
			for (int i=0; i<m->port_in.num; i++){
				float y = module_port_in_y(m, i);
				float x = module_port_in_x(m);
				if (abs(x - mx) < 10 and abs(y - my) < 10){
					s.type = Selection::TYPE_PORT_IN;
					s.module = m;
					s.port = i;
					s.port_type = m->port_in[i].type;
					s.dx = x;
					s.dy = y;
					return s;
				}
			}
			for (int i=0; i<m->port_out.num; i++){
				float y = module_port_out_y(m, i);
				float x = module_port_out_x(m);
				if (abs(x - mx) < 10 and abs(y - my) < 10){
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


	color signal_color(SignalType type, bool hover = false)
	{
		if (type == SignalType::AUDIO)
			return hover ? view->colors.red_hover : view->colors.red;
		if (type == SignalType::MIDI)
			return hover ? view->colors.green_hover : view->colors.green;
		if (type == SignalType::BEATS)
			return hover ? view->colors.blue_hover : view->colors.blue;
		return hover ? view->colors.white_hover : view->colors.white;
	}

	void draw_cable(Painter *p, SignalChain::Cable *c)
	{
		complex p0 = complex(module_port_out_x(c->source), module_port_out_y(c->source, c->source_port));
		complex p1 = complex(module_port_in_x(c->target), module_port_in_y(c->target, c->target_port));

		float length = (p1 - p0).abs();
		Interpolator<complex> inter(Interpolator<complex>::TYPE_CUBIC_SPLINE);
		inter.add2(p0, complex(length,0));
		inter.add2(p1, complex(length,0));

		p->set_color(signal_color(c->type));

		complex qq;
		for (float t=0; t<1.0f; t+=0.025f){
			complex q = inter.get(t);
			if (t > 0)
				p->draw_line(qq.x, qq.y, q.x, q.y);
			qq = q;
		}
		complex m = inter.get(0.5f);
		complex d = inter.getTang(0.5);
		d /= d.abs();
		complex e = d * c_i;
		Array<complex> pp;
		float arrow_length = min(length / 7, 18.0f);
		pp.add(m + d * arrow_length);
		pp.add(m - d * arrow_length + e * arrow_length / 2);
		pp.add(m - d * arrow_length - e * arrow_length / 2);
		p->draw_polygon(pp);
		//p->dr
	}

	void draw_module(Painter *p, Module *m)
	{
		p->set_color(view->colors.background_track_selected);
		if (hover.type == Selection::TYPE_MODULE and hover.module == m)
			p->set_color(ColorInterpolate(view->colors.background_track_selected, view->colors.hover, 0.25f));
		p->set_roundness(view->CORNER_RADIUS);
		p->draw_rect(module_rect(m));
		p->set_roundness(0);
		if (sel.type == sel.TYPE_MODULE and sel.module == m){
			p->set_color(view->colors.text);
			p->set_font("", 12, true, false);
		}else{
			p->set_color(view->colors.text_soft1);
		}
		string type = module_header(m);
		float ww = p->get_str_width(type);
		p->draw_str(m->module_x + MODULE_WIDTH/2 - ww/2, m->module_y + 4, type);
		p->set_font("", 12, false, false);
	}

	void draw_ports(Painter *p, Module *m)
	{
		foreachi(auto &pd, m->port_in, i){
			bool hovering = (hover.type == Selection::TYPE_PORT_IN and hover.module == m and hover.port == i);
			p->set_color(signal_color(pd.type, hovering));
			float r = hovering ? 6 : 4;
			p->draw_circle(module_port_in_x(m), module_port_in_y(m, i), r);
		}
		foreachi(auto *pd, m->port_out, i){
			bool hovering = (hover.type == Selection::TYPE_PORT_OUT and hover.module == m and hover.port == i);
			p->set_color(signal_color(pd->type, hovering));
			float r = hovering ? 6 : 4;
			p->draw_circle(module_port_out_x(m), module_port_out_y(m, i), r);
		}
	}

	void on_draw(Painter* p)
	{
		int w = p->width;
		int h = p->height;
		p->set_color(view->colors.background);
		p->draw_rect(0, 0, w, h);
		p->set_font_size(12);

		for (auto *m: chain->modules)
			draw_module(p, m);

		for (auto *c: chain->cables)
			draw_cable(p, c);

		for (auto *m: chain->modules)
			draw_ports(p, m);

		for (auto &pp: chain->_ports_out){
			p->set_color(Red);
			p->draw_circle(module_port_out_x(pp.module)+20, module_port_out_y(pp.module, pp.port), 10);
		}

		if (sel.type == sel.TYPE_PORT_IN or sel.type == sel.TYPE_PORT_OUT){
			p->set_color(White);
			if (hover.target_module){
				p->set_line_width(5);
				Module *t = hover.target_module;
				if (hover.type == hover.TYPE_PORT_IN)
					p->draw_line(sel.dx, sel.dy, module_port_out_x(t), module_port_out_y(t, hover.target_port));
				else
					p->draw_line(sel.dx, sel.dy, module_port_in_x(t), module_port_in_y(t, hover.target_port));
				p->set_line_width(1);
			}else
				p->draw_line(sel.dx, sel.dy, hui::GetEvent()->mx, hui::GetEvent()->my);
		}


		float mx = hui::GetEvent()->mx;
		float my = hui::GetEvent()->my;
		Selection hh = get_hover(mx, my);
		if (hh.type == hover.TYPE_PORT_IN)
			AudioView::draw_cursor_hover(p, _("input: ") + signal_type_name(hh.port_type), mx, my, p->area());
		if (hh.type == hover.TYPE_PORT_OUT)
			AudioView::draw_cursor_hover(p, _("output: ") + signal_type_name(hh.port_type), mx, my, p->area());
	}

	void on_chain_update()
	{
		redraw("area");
	}

	void on_left_button_down()
	{
		float mx = hui::GetEvent()->mx;
		float my = hui::GetEvent()->my;
		hover = get_hover(mx, my);
		sel = hover;
		apply_sel();
		if (sel.type == sel.TYPE_PORT_IN){
			chain->disconnect_target(sel.module, sel.port);
		}else if (sel.type == sel.TYPE_PORT_OUT){
			chain->disconnect_source(sel.module, sel.port);
		}
		redraw("area");
	}

	void on_left_button_up()
	{
		if (sel.type == sel.TYPE_PORT_IN or sel.type == sel.TYPE_PORT_OUT){
			if (hover.target_module){
				if (sel.type == sel.TYPE_PORT_IN){
					chain->disconnect_source(hover.target_module, hover.target_port);
					chain->connect(hover.target_module, hover.target_port, sel.module, sel.port);
				}else if (sel.type == sel.TYPE_PORT_OUT){
					chain->disconnect_target(hover.target_module, hover.target_port);
					chain->connect(sel.module, sel.port, hover.target_module, hover.target_port);
				}
			}
			sel = Selection();
			apply_sel();
		}
		redraw("area");
	}

	void on_mouse_move()
	{
		float mx = hui::GetEvent()->mx;
		float my = hui::GetEvent()->my;
		if (hui::GetEvent()->lbut){
			if (sel.type == sel.TYPE_MODULE){
				auto *m = sel.module;
				m->module_x = mx + sel.dx;
				m->module_y = my + sel.dy;
			}else if (sel.type == sel.TYPE_PORT_IN or sel.type == sel.TYPE_PORT_OUT){
				hover.target_module = nullptr;
				auto h = get_hover(mx, my);
				if (h.module != sel.module and h.port_type == sel.port_type){
					if (h.type == sel.TYPE_PORT_IN and sel.type == sel.TYPE_PORT_OUT){
						hover.target_module = h.module;
						hover.target_port = h.port;
					}
					if (h.type == sel.TYPE_PORT_OUT and sel.type == sel.TYPE_PORT_IN){
						hover.target_module = h.module;
						hover.target_port = h.port;
					}
				}
			}
		}else{
			hover = get_hover(mx, my);
		}
		redraw("area");
	}

	void on_right_button_down()
	{
		float mx = hui::GetEvent()->mx;
		float my = hui::GetEvent()->my;
		hover = get_hover(mx, my);
		sel = hover;
		apply_sel();

		if (hover.type == hover.TYPE_MODULE){
			editor->menu_module->open_popup(this);
		}else if (hover.type < 0){
			editor->menu_chain->open_popup(this);
		}
	}

	void on_key_down()
	{
		int key = hui::GetEvent()->key_code;
		if (key == hui::KEY_DELETE){
			if (sel.type == sel.TYPE_MODULE){
				chain->remove(sel.module);
				hover = sel = Selection();
			}
		}
	}

	void apply_sel()
	{
		session->win->side_bar->module_console->setModule(sel.module);
		editor->show_config(sel.module);
	}

	void on_activate()
	{
	}

	void on_delete()
	{
		hui::RunLater(0.001f, [&](){ editor->delete_chain(chain); });
	}



	void on_add_audio_source()
	{
		string name = session->plugin_manager->choose_module(win, session, ModuleType::AUDIO_SOURCE);
		if (name.num > 0){
			auto *m = chain->addAudioSource(name);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void on_add_audio_effect()
	{
		string name = session->plugin_manager->choose_module(win, session, ModuleType::AUDIO_EFFECT);
		if (name.num > 0){
			auto *m = chain->addAudioEffect(name);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void on_add_audio_visualizer()
	{
		string name = session->plugin_manager->choose_module(win, session, ModuleType::AUDIO_VISUALIZER);
		if (name.num > 0){
			auto *m = chain->addAudioVisualizer(name);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void on_add_audio_joiner()
	{
		auto *m = chain->addAudioJoiner();
		m->module_x = sel.dx;
		m->module_y = sel.dy;
	}

	void on_add_audio_sucker()
	{
		auto *m = chain->addAudioSucker();
		m->module_x = sel.dx;
		m->module_y = sel.dy;
	}

	void on_add_audio_input_stream()
	{
		auto *m = chain->addAudioInputStream();
		m->module_x = sel.dx;
		m->module_y = sel.dy;
	}

	void on_add_midi_source()
	{
		string name = session->plugin_manager->choose_module(win, session, ModuleType::MIDI_SOURCE);
		if (name.num > 0){
			auto *m = chain->addMidiSource(name);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void on_add_midi_effect()
	{
		string name = session->plugin_manager->choose_module(win, session, ModuleType::MIDI_EFFECT);
		if (name.num > 0){
			auto *m = chain->addMidiEffect(name);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void on_add_synthesizer()
	{
		string name = session->plugin_manager->choose_module(win, session, ModuleType::SYNTHESIZER);
		if (name.num > 0){
			auto *m = chain->addSynthesizer(name);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void on_add_midi_input_stream()
	{
		auto *m = chain->addMidiInputStream();
		m->module_x = sel.dx;
		m->module_y = sel.dy;
	}

	void on_add_pitch_detector()
	{
		auto *m = chain->addPitchDetector();
		m->module_x = sel.dx;
		m->module_y = sel.dy;
	}

	void on_add_beat_source()
	{
		string name = session->plugin_manager->choose_module(win, session, ModuleType::BEAT_SOURCE);
		if (name.num > 0){
			auto *m = chain->addBeatSource(name);
			m->module_x = sel.dx;
			m->module_y = sel.dy;
		}
	}

	void on_add_beat_midifier()
	{
		auto *m = chain->addBeatMidifier();
		m->module_x = sel.dx;
		m->module_y = sel.dy;
	}

	void on_reset()
	{
		chain->reset();
	}

	/*void on_load()
	{
		if (hui::FileDialogOpen(win, "", "", "*.chain", "*.chain"))
			chain->load(hui::Filename);
	}*/

	void on_save()
	{
		if (hui::FileDialogSave(win, _("Save the signal chain"), session->storage->current_chain_directory, "*.chain", "*.chain")){
			session->storage->current_chain_directory = hui::Filename.dirname();
			chain->save(hui::Filename);
		}
	}



	void on_module_delete()
	{
		if (sel.type == sel.TYPE_MODULE){
			chain->remove(sel.module);
			hover = sel = Selection();
			apply_sel();
		}
	}

	void on_module_configure()
	{
		apply_sel();
		session->win->side_bar->open(SideBar::MODULE_CONSOLE);
	}

};


SignalEditor::SignalEditor(Session *session) :
	BottomBar::Console(_("Signal Chain"), session)
{
	grid_id = "main-grid";
	config_grid_id = "config-panel-grid";
	addGrid("", 0, 0, grid_id);
	setTarget(grid_id);
	addTabControl("!left\\aaa", 0, 0, "selector");
	addGrid("!width=380,noexpandx", 1, 0, config_grid_id);
	setTarget(config_grid_id);
	addLabel("!bold,center,big,expandx", 0, 0, "config-label");
	addLabel("!bold,center,expandx", 0, 1, "message");

	menu_chain = hui::CreateResourceMenu("popup_signal_chain_menu");
	menu_module = hui::CreateResourceMenu("popup_signal_module_menu");

	config_module = nullptr;
	config_panel = nullptr;

	event("selector", [&]{ on_chain_switch(); });

	add_chain(session->signal_chain);
	show_config(nullptr);
}

SignalEditor::~SignalEditor()
{
	show_config(nullptr);
	delete menu_chain;
	delete menu_module;
}

void SignalEditor::add_chain(SignalChain *c)
{
	auto *tab = new SignalEditorTab(this, c);
	int index = tabs.num;
	string grid_id = "grid-" + i2s(index);
	if (index > 0)
		addString("selector", c->name);
	else
		changeString("selector", index, c->name);
	setTarget("selector");
	addGrid("", index, 0, grid_id);
	embed(tab, grid_id, 0, 0);
	tabs.add(tab);

	setInt("selector", index);
}

void SignalEditor::on_new()
{
	add_chain(new SignalChain(session, "new"));
}

void SignalEditor::on_load()
{
	if (hui::FileDialogOpen(win, _("Load a signal chain"), session->storage->current_chain_directory, "*.chain", "*.chain")){
		session->storage->current_chain_directory = hui::Filename.dirname();
		auto *c = new SignalChain(session, "new");
		c->load(hui::Filename);
		add_chain(c);
	}
}

void SignalEditor::on_chain_switch()
{
	//msg_write("switch");
}

void SignalEditor::show_config(Module *m)
{
	if (m and (m == config_module))
		return;
	if (config_panel)
		delete config_panel;
	config_panel = nullptr;
	config_module = m;
	if (m){
		setString("config-label", module_header(m));
		config_panel = m->create_panel();
		if (config_panel){
			embed(config_panel, config_grid_id, 0, 2);
			config_panel->set_large(false);
			//setOptions(config_grid_id, "width=330,noexpandx");
			hideControl("message", true);
		}else{
			setString("message", _("module not configurable"));
			hideControl("message", false);
		}
	}else{
		setString("config-label", "");
		setString("message", _("no module selected"));
		hideControl("message", false);
	}
}

void SignalEditor::delete_chain(SignalChain *c)
{
	foreachi(auto *t, tabs, i)
		if (t->chain == c){
			if (i == 0){
				session->e(_("not allowed to delete the main signal chain"));
			}else{
				delete t;
				delete c;
				tabs.erase(i);
				removeString("selector", i);
				setInt("selector", 0);
			}
		}
}
