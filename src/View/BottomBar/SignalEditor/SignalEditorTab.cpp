/*
 * SignalEditorTab.cpp
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#include "SignalEditorTab.h"
#include "SignalEditorBackground.h"
#include "SignalEditorModule.h"
#include "SignalEditorCable.h"
#include "../SignalEditor.h"
#include "../../AudioView.h"
#include "../../HoverData.h"
#include "../../../Module/Port/Port.h"
#include "../../../Module/Module.h"
#include "../../../Module/SignalChain.h"
#include "../../Helper/Graph/SceneGraph.h"
#include "../../Helper/Graph/Node.h"
#include "../../Helper/Graph/Scrollable.h"
#include "../../Helper/Graph/ScrollBar.h"
#include "../../Helper/Drawing.h"
#include "../../MouseDelayPlanner.h"
#include "../../../Data/base.h"
#include "../../../Session.h"
#include "../../../Storage/Storage.h"
#include "../../../Plugins/PluginManager.h"




class SignalEditorPlayButton : public scenegraph::NodeRel {
public:
	SignalEditorTab *tab;
	SignalChain *chain;
	SignalEditorPlayButton(SignalEditorTab *t) : scenegraph::NodeRel({32, -32}, 20, 20) {
		align.dz = 30;
		align.vertical = AlignData::Mode::BOTTOM;
		set_perf_name("button");
		tab = t;
		chain = tab->chain;
	}
	void on_draw(Painter *p) override {
		color col = theme.text_soft2;
		if (!chain->is_active()) {
			col = theme.text_soft1;
			if (this->is_cur_hover())
				col = theme.text;
		}
		p->set_color(col);
		p->set_font_size(18);
		p->draw_str({area.x1, area.y1}, u8"\u25B6");
	}
	bool on_left_button_down(const vec2 &m) override {
		chain->start();
		return true;
	}
	string get_tip() const override {
		return _("start");
	}
};

class SignalEditorStopButton : public scenegraph::NodeRel {
public:
	SignalEditorTab *tab;
	SignalChain *chain;
	SignalEditorStopButton(SignalEditorTab *t) : scenegraph::NodeRel({72, -32}, 20, 20) {
		align.dz = 30;
		align.vertical = AlignData::Mode::BOTTOM;
		set_perf_name("button");
		tab = t;
		chain = tab->chain;
	}
	void on_draw(Painter *p) override {
		color col = theme.text_soft2;
		if (chain->is_active()) {
			col = theme.text_soft1;
			if (this->is_cur_hover())
				col = theme.text;
		}
		p->set_color(col);
		p->set_font_size(18);
		p->draw_str({area.x1, area.y1}, u8"\u23F9");
	}
	bool on_left_button_down(const vec2 &m) override {
		chain->stop();
		return true;
	}
	string get_tip() const override {
		return _("stop");
	}
};


SignalEditorTab::SignalEditorTab(SignalEditor *ed, SignalChain *_chain) {
	add_grid("", 0, 0, "grid");
	set_target("grid");
	add_drawing_area("!expandx,expandy,grabfocus", 0, 0, "area");

	editor = ed;
	view = ed->view;
	session = ed->session;
	chain = _chain;

	pad = new ScrollPad();
	pad->align.dz = 5;
	graph = scenegraph::SceneGraph::create_integrated(this, "area", pad, "SignalEditor", [=] (Painter *p) {
		p->set_font_size(theme.FONT_SIZE);
		graph->update_geometry_recursive(p->area());
		pad->_update_scrolling();
		graph->draw(p);

		/*for (auto &pp: chain->_ports_out){
			p->set_color(Red);
			p->draw_circle(module_port_out_x(pp.module)+20, module_port_out_y(pp.module, pp.port), 10);
		}*/


		auto m = hui::GetEvent()->m;

		string tip;
		if (graph->hover.node)
			tip = graph->hover.node->get_tip();
		if (tip.num > 0) {
			p->set_font_size(theme.FONT_SIZE);
			draw_cursor_hover(p, tip, m, graph->area);
		}
	});
	background = new SignalEditorBackground(this);
	graph->add_child(background);
	pad->connect_scrollable(background);
	graph->add_child(new SignalEditorPlayButton(this));
	graph->add_child(new SignalEditorStopButton(this));

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

	chain->subscribe(this, [=]{ on_chain_update(); }, chain->MESSAGE_STATE_CHANGE);
	//chain->subscribe(this, [=]{ on_chain_update(); }, chain->MESSAGE_PLAY_END_OF_STREAM);
	chain->subscribe(this, [=]{ on_chain_update(); }, chain->MESSAGE_DELETE_CABLE);
	chain->subscribe(this, [=]{ on_chain_update(); }, chain->MESSAGE_DELETE_MODULE);
	chain->subscribe(this, [=]{ on_chain_update(); }, chain->MESSAGE_ADD_CABLE);
	chain->subscribe(this, [=]{ on_chain_update(); }, chain->MESSAGE_ADD_MODULE);
	chain->subscribe(this, [=]{ on_chain_delete(); }, chain->MESSAGE_DELETE);

	on_chain_update();
}
SignalEditorTab::~SignalEditorTab() {
	chain->unsubscribe(this);
}

color SignalEditorTab::signal_color_base(SignalType type) {
	if (type == SignalType::AUDIO)
		return theme.red;
	if (type == SignalType::MIDI)
		return theme.green;
	if (type == SignalType::BEATS)
		return theme.blue;
	return theme.white;
}

color SignalEditorTab::signal_color(SignalType type, bool hover) {
	color c = signal_color_base(type);
	c = color::interpolate(c, theme.text, 0.2f);
	//c = color::interpolate(c, colors.background, 0.2f);
	if (hover)
		c = theme.hoverify(c);
	return c;
}

void SignalEditorTab::draw_arrow(Painter *p, const vec2 &m, const vec2 &_d, float length) {
	vec2 d = _d / _d.length();
	vec2 e = vec2(d.y,-d.x);
	Array<vec2> pp;
	pp.add(m + d * length);
	pp.add(m - d * length + e * length / 2);
	pp.add(m - d * length * 0.8f);
	pp.add(m - d * length - e * length / 2);
	p->draw_polygon(pp);
}


void SignalEditorTab::on_draw(Painter* p) {
	p->set_font_size(theme.FONT_SIZE);
	graph->update_geometry_recursive(p->area());
	pad->_update_scrolling();
	graph->draw(p);

	/*for (auto &pp: chain->_ports_out){
		p->set_color(Red);
		p->draw_circle(module_port_out_x(pp.module)+20, module_port_out_y(pp.module, pp.port), 10);
	}*/


	auto m = hui::GetEvent()->m;


	string tip;
	if (graph->hover.node)
		tip = graph->hover.node->get_tip();
	if (tip.num > 0) {
		p->set_font_size(theme.FONT_SIZE);
		draw_cursor_hover(p, tip, m, graph->area);
	}
}

void SignalEditorTab::on_chain_update() {
	// delete old modules
	Array<SignalEditorModule*> to_del;
	for (auto m: modules)
		if (weak(chain->modules).find(m->module) < 0)
			to_del.add(m);
	for (auto m: to_del) {
		modules.erase(modules.find(m));
		pad->delete_child(m);
	}

	// add new modules
	for (auto m: weak(chain->modules))
		if (!get_module(m)) {
			auto mm = new SignalEditorModule(this, m);
			modules.add(mm);
			pad->add_child(mm);
		}

	// cables
	for (auto c: cables)
		pad->delete_child(c);
	cables.clear();
	for (auto c: chain->cables()) {
		auto cc = new SignalEditorCable(this, c);
		cables.add(cc);
		pad->add_child(cc);
	}

	graph->hover = HoverData();


	update_module_positions();
}

void SignalEditorTab::on_chain_delete() {
	editor->remove_tab(this);
}

void SignalEditorTab::popup_chain() {
	editor->menu_chain->open_popup(this);
}

void SignalEditorTab::popup_module() {
	editor->menu_module->open_popup(this);
}

void SignalEditorTab::on_key_down() {
	int key = hui::GetEvent()->key_code;

	if (key == hui::KEY_DELETE)
		on_module_delete();
}

void SignalEditorTab::on_activate() {
}

void SignalEditorTab::on_delete() {
	if (chain->belongs_to_system) {
		session->e(_("not allowed to delete the main signal chain"));
		return;
	}
	hui::RunLater(0.001f, [=](){ chain->unregister(); });
}


void SignalEditorTab::on_add(ModuleCategory type) {
	auto names = session->plugin_manager->find_module_sub_types(type);
	if (names.num > 1) {
		string name = session->plugin_manager->choose_module(win, session, type);
		if (name.num > 0) {
			auto *m = chain->add(type, name);
			m->module_x = graph->m.x;
			m->module_y = graph->m.y;
		}
	} else {
		auto *m = chain->add(type);
		m->module_x = graph->m.x;
		m->module_y = graph->m.y;
	}
	update_module_positions();
}

void SignalEditorTab::on_reset() {
	chain->reset(false);
}

/*void SignalEditorTab::on_load() {
	if (hui::FileDialogOpen(win, "", "", "*.chain", "*.chain"))
		chain->load(hui::Filename);
}*/

void SignalEditorTab::on_save() {
	hui::file_dialog_save(win, session->storage->current_chain_directory, {"title=" + _("Save the signal chain"), "filter=*.chain", "showfilter=*.chain"}, [this](const Path &filename) {
		if (!filename.is_empty()) {
			session->storage->current_chain_directory = filename.parent();
			chain->save(filename);
		}
	});
}



void SignalEditorTab::on_module_delete() {
	for (auto m: sel_modules)
		chain->delete_module(m);
	select_module(nullptr);
}

void SignalEditorTab::on_module_configure() {
}

void SignalEditorTab::select_module(Module *m, bool add) {
	if (!add)
		sel_modules.clear();
	if (m)
		sel_modules.add(m);
	editor->show_config(m);
	redraw("area");
}

void SignalEditorTab::update_module_positions() {
	// temporarily set a 1:1 coord system
	pad->Node::update_geometry_recursive(pad->area);
	rect r = rect::EMPTY;
	if (modules.num > 0)
		r = modules[0]->area;

	for (auto *m: modules) {
		m->align.dx = m->module->module_x;
		m->align.dy = m->module->module_y;
		r = r || m->area;
	}
	pad->set_content(r);
}

SignalEditorModule *SignalEditorTab::get_module(Module *m) {
	for (auto mm: modules)
		if (mm->module == m)
			return mm;
	return nullptr;
}

