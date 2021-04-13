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
#include "../../Graph/ScrollBar.h"
#include "../../MouseDelayPlanner.h"
#include "../../../Data/base.h"
#include "../../../Session.h"
#include "../../../Storage/Storage.h"
#include "../../../Plugins/PluginManager.h"




const float MODULE_WIDTH = 140;
const float MODULE_HEIGHT = 23;
const float MODULE_GRID = 23;

string module_header(Module *m) {
	if (m->module_name.num > 0)
		return m->module_name;
	if (m->module_class.num > 0)
		return m->module_class;
	return m->category_to_str(m->module_category);
}


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



class SignalEditorPlayButton : public scenegraph::NodeRel {
public:
	SignalEditorTab *tab;
	SignalChain *chain;
	SignalEditorPlayButton(SignalEditorTab *t) : scenegraph::NodeRel(50, -50, 20, 20) {
		align.dz = 30;
		align.vertical = AlignData::Mode::BOTTOM;
		tab = t;
		chain = tab->chain;
	}
	void on_draw(Painter *p) override {
		p->set_color(tab->view->colors.text_soft2);
		if (this->is_cur_hover())
			p->set_color(tab->view->colors.text);
		p->set_font_size(20);
		p->draw_str(area.x1, area.y1, chain->is_active() ? u8"\u23F9" : u8"\u25B6");
	}
	bool on_left_button_down(float mx, float my) override {
		if (chain->is_active())
			chain->stop();
		else
			chain->start();
		return true;
	}
	string get_tip() override {
		if (chain->is_active())
			return _("stop");
		return _("start");
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

	graph = new scenegraph::SceneGraph([&]{});
	auto hbox = new scenegraph::VBox;
	graph->add_child(hbox);
	background = new SignalEditorBackground(this);
	hbox->add_child(background);
	scroll_bar_h = new ScrollBarHorizontal();
	scroll_bar_h->constrained = false;
	scroll_bar_h->update(1,1);
	scroll_bar_h->hidden = true;
	scroll_bar_h->set_callback([=]{
		//move_cam()
	});
	hbox->add_child(scroll_bar_h);
	graph->add_child(new SignalEditorPlayButton(this));

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
		return view->colors.red;
	if (type == SignalType::MIDI)
		return view->colors.green;
	if (type == SignalType::BEATS)
		return view->colors.blue;
	return view->colors.white;
}

color SignalEditorTab::signal_color(SignalType type, bool hover) {
	color c = signal_color_base(type);
	c = color::interpolate(c, view->colors.text, 0.2f);
	//c = color::interpolate(c, view->colors.background, 0.2f);
	if (hover)
		c = view->colors.hoverify(c);
	return c;
}

void SignalEditorTab::draw_arrow(Painter *p, const complex &m, const complex &_d, float length) {
	complex d = _d / _d.abs();
	complex e = d * complex::I;
	Array<complex> pp;
	pp.add(m + d * length);
	pp.add(m - d * length + e * length / 2);
	pp.add(m - d * length * 0.8f);
	pp.add(m - d * length - e * length / 2);
	p->draw_polygon(pp);
}


void SignalEditorTab::on_draw(Painter* p) {
	p->set_font_size(view->FONT_SIZE);
	graph->update_geometry_recursive(p->area());
	graph->on_draw(p);

	for (auto &pp: chain->_ports_out){
		p->set_color(Red);
		p->draw_circle(module_port_out_x(pp.module)+20, module_port_out_y(pp.module, pp.port), 10);
	}


	float mx = hui::GetEvent()->mx;
	float my = hui::GetEvent()->my;


	string tip;
	if (graph->hover.node)
		tip = graph->hover.node->get_tip();
	if (tip.num > 0) {
		p->set_font_size(view->FONT_SIZE);
		AudioView::draw_cursor_hover(p, tip, mx, my, graph->area);
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
		background->delete_child(m);
	}

	// add new modules
	for (auto m: weak(chain->modules))
		if (!get_module(m)) {
			auto mm = new SignalEditorModule(this, m);
			modules.add(mm);
			background->add_child(mm);
		}

	// cables
	for (auto c: cables)
		background->delete_child(c);
	cables.clear();
	for (auto c: chain->cables()) {
		auto cc = new SignalEditorCable(this, c);
		cables.add(cc);
		background->add_child(cc);
	}

	graph->hover = HoverData();


	redraw("area");
}

void SignalEditorTab::on_chain_delete() {
	editor->remove_tab(this);
}

void SignalEditorTab::on_left_button_down() {
	float mx = hui::GetEvent()->mx;
	float my = hui::GetEvent()->my;
	graph->on_left_button_down(mx, my);
	redraw("area");
}

void SignalEditorTab::on_left_button_up() {
	float mx = hui::GetEvent()->mx;
	float my = hui::GetEvent()->my;
	graph->on_left_button_up(mx, my);
	redraw("area");
}

void SignalEditorTab::on_mouse_move() {
	float mx = hui::GetEvent()->mx;
	float my = hui::GetEvent()->my;
	graph->on_mouse_move(mx, my);
	redraw("area");
}

void SignalEditorTab::on_right_button_down() {
	float mx = hui::GetEvent()->mx;
	float my = hui::GetEvent()->my;
	graph->on_right_button_down(mx, my);
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

	if (key == hui::KEY_UP)
		move_cam(0, 10);
	if (key == hui::KEY_DOWN)
		move_cam(0, -10);
	if (key == hui::KEY_LEFT)
		move_cam(10, 0);
	if (key == hui::KEY_RIGHT)
		move_cam(-10, 0);
}

void SignalEditorTab::move_cam(float dx, float dy) {
	// TODO restrict...
	view_offset.x -= dx;
	view_offset.y -= dy;
	scroll_bar_h->offset = view_offset.x;
	scroll_bar_h->update(graph->area.width(), 1000);
	redraw("area");
}

void SignalEditorTab::on_mouse_wheel() {
	float dx = hui::GetEvent()->scroll_x;
	float dy = hui::GetEvent()->scroll_y;
	move_cam(dx*10, dy*10);
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
	Array<string> names = session->plugin_manager->find_module_sub_types(type);
	if (names.num > 1) {
		string name = session->plugin_manager->choose_module(win, session, type);
		if (name.num > 0) {
			auto *m = chain->add(type, name);
			m->module_x = graph->mx;
			m->module_y = graph->my;
		}
	} else {
		auto *m = chain->add(type);
		m->module_x = graph->mx;
		m->module_y = graph->my;
	}
}

void SignalEditorTab::on_reset() {
	chain->reset(false);
}

/*void SignalEditorTab::on_load() {
	if (hui::FileDialogOpen(win, "", "", "*.chain", "*.chain"))
		chain->load(hui::Filename);
}*/

void SignalEditorTab::on_save() {
	if (hui::FileDialogSave(win, _("Save the signal chain"), session->storage->current_chain_directory, "*.chain", "*.chain")) {
		session->storage->current_chain_directory = hui::Filename.parent();
		chain->save(hui::Filename);
	}
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
	for (auto *m: weak(background->children)) {
		m->align.dx = ((SignalEditorModule*)m)->module->module_x;
		m->align.dy = ((SignalEditorModule*)m)->module->module_y;
	}
	redraw("area");
}

SignalEditorModule *SignalEditorTab::get_module(Module *m) {
	for (auto mm: modules)
		if (mm->module == m)
			return mm;
	return nullptr;
}

