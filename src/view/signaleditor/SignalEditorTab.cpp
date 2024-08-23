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
#include "../audioview/AudioView.h"
#include "../mainview/MainView.h"
#include "../HoverData.h"
#include "../dialog/ModuleSelectorDialog.h"
#include "../helper/graph/SceneGraph.h"
#include "../helper/graph/Node.h"
#include "../helper/Drawing.h"
#include "../sidebar/SideBar.h"
#include "../MouseDelayPlanner.h"
#include "../TsunamiWindow.h"
#include "../ColorScheme.h"
#include "../../module/port/Port.h"
#include "../../module/Module.h"
#include "../../module/SignalChain.h"
#include "../../data/base.h"
#include "../../Session.h"
#include "../../storage/Storage.h"
#include "../../plugins/PluginManager.h"
#include "../../lib/image/Painter.h"
#include "../../lib/hui/language.h"
#include "../../lib/hui/common_dlg.h"


namespace tsunami {

class SignalEditorPlayButton : public scenegraph::NodeRel {
public:
	SignalEditorTab *tab;
	SignalChain *chain;
	SignalEditorPlayButton(SignalEditorTab *t) : scenegraph::NodeRel({-32, -32}, 45, 45) {
		align.dz = 30;
		align.horizontal = AlignData::Mode::Right;
		align.vertical = AlignData::Mode::Bottom;
		set_perf_name("button");
		tab = t;
		chain = tab->chain;
	}
	void on_draw(Painter *p) override {
		color bg = theme.background_overlay;
		color fg = theme.text_soft3;
		float radius = area.width() * 0.48f;
		if (is_cur_hover()) {
			bg = theme.hoverify(bg);
			fg = theme.text;
			radius = area.width() * 0.6f;
		}
		p->set_color(bg);
		p->draw_circle(area.center(), radius);
		p->set_color(fg);
		p->set_font_size(theme.FONT_SIZE_HUGE);
		if (chain->is_active())
			draw_str_centered(p, area.center(), u8"\u23F8");
		else
			draw_str_centered(p, area.center(), u8"\u25B6");
		p->set_font_size(theme.FONT_SIZE);
	}
	bool on_left_button_down(const vec2 &m) override {
		if (chain->is_active())
			chain->stop();
		else
			chain->start();
		return true;
	}
	string get_tip() const override {
		return _("start");
	}
};


class SignalEditorBigButton : public scenegraph::NodeRel {
public:
	SignalEditorTab *tab;
	SignalChain *chain;
	explicit SignalEditorBigButton(SignalEditorTab *t) : scenegraph::NodeRel({-32, 32}, 45, 45) {
		align.dz = 30;
		align.horizontal = AlignData::Mode::Right;
		align.vertical = AlignData::Mode::Top;
		set_perf_name("button");
		tab = t;
		chain = tab->chain;
	}
	void on_draw(Painter *p) override {
		color bg = theme.background_overlay;
		color fg = theme.text_soft3;
		float radius = area.width() * 0.48f;
		if (is_cur_hover()) {
			bg = theme.hoverify(bg);
			fg = theme.text;
			radius = area.width() * 0.6f;
		}
		p->set_color(bg);
		p->draw_circle(area.center(), radius);
		p->set_color(fg);
		p->set_font_size(theme.FONT_SIZE_HUGE);
		draw_str_centered(p, area.center(), "+");
		p->set_font_size(theme.FONT_SIZE);
	}
	bool on_left_button_down(const vec2 &m) override {
		chain->session->main_view->open_for(chain);
		return true;
	}
	string get_tip() const override {
		return _("edit in main view");
	}
};

SignalEditorTab::SignalEditorTab(SignalChain *_chain) {

	chain = _chain;
	session = chain->session;
	view = session->view;

	pad = new ScrollPad();
	pad->scale_min = 0.2f;
	pad->scale_max = 2.0f;
	add_child(pad);
	background = new SignalEditorBackground(this);
	pad->add_child(background);
	pad->connect_scrollable(background);
	//add_child(new SignalEditorPlayButton(this));

	/*
	event("signal_chain_add_audio_source", [this] { on_add(ModuleCategory::AUDIO_SOURCE); });
	event("signal_chain_add_audio_effect", [this] { on_add(ModuleCategory::AUDIO_EFFECT); });
	event("signal_chain_add_stream", [this] { on_add(ModuleCategory::STREAM); });
	event("signal_chain_add_plumbing", [this] { on_add(ModuleCategory::PLUMBING); });
	event("signal_chain_add_audio_visualizer", [this] { on_add(ModuleCategory::AUDIO_VISUALIZER); });
	event("signal_chain_add_midi_source", [this] { on_add(ModuleCategory::MIDI_SOURCE); });
	event("signal_chain_add_midi_effect", [this] { on_add(ModuleCategory::MIDI_EFFECT); });
	event("signal_chain_add_synthesizer", [this] { on_add(ModuleCategory::SYNTHESIZER); });
	event("signal_chain_add_pitch_detector", [this] { on_add(ModuleCategory::PITCH_DETECTOR); });
	event("signal_chain_add_beat_source", [this] { on_add(ModuleCategory::BEAT_SOURCE); });
	event("signal_chain_reset", [this] { on_reset(); });
	event("signal_chain_activate", [this] { on_activate(); });
	event("signal_chain_delete", [this] { on_delete(); });
	event("signal_chain_save", [this] { on_save(); });
	event("signal_module_delete", [this] { on_module_delete(); });
	event("signal_module_configure", [this] { on_module_configure(); });

	event("signal_chain_new", [this] { editor->on_new(); });
	event("signal_chain_load", [this] { editor->on_load(); });*/

	chain->out_death >> create_sink([this] { out_delete_me(); });

	chain->out_state_changed >> create_sink([this] { on_chain_update(); });
	//chain->subscribe(this, [this] { on_chain_update(); }, chain->MESSAGE_PLAY_END_OF_STREAM);
	chain->out_delete_cable >> create_sink([this] { on_chain_update(); });
	chain->out_delete_module >> create_sink([this] { on_chain_update(); });
	chain->out_add_cable >> create_sink([this] { on_chain_update(); });
	chain->out_add_module >> create_sink([this] { on_chain_update(); });

	//menu_chain = hui::create_resource_menu("popup_signal_chain_menu", this);
	//menu_module = hui::create_resource_menu("popup_signal_module_menu", this);

	on_chain_update();
}

SignalEditorTab::~SignalEditorTab() {
	chain->unsubscribe(this);
}

void* SignalEditorTab::mvn_data() const {
	return chain;
}

string SignalEditorTab::mvn_description() const {
	return "chain: " + chain->name;
}

void SignalEditorTab::mvn_on_enter() {
	session->side_bar()->open(SideBar::Index::SignalChainConsole);
}

void SignalEditorTab::play() {
	chain->start();
}

void SignalEditorTab::stop() {
	chain->stop_hard();
}

void SignalEditorTab::pause() {
	chain->stop();
}

bool SignalEditorTab::is_playback_active() {
	return chain->is_active() or chain->is_prepared();
}

bool SignalEditorTab::is_paused() {
	return chain->is_prepared() and !chain->is_active();
}

bool SignalEditorTab::mvn_can_play() {
	return true;
}

bool SignalEditorTab::mvn_can_pause() {
	return chain->is_active();
}

bool SignalEditorTab::mvn_can_stop() {
	return chain->is_active() or chain->is_prepared();
}

color SignalEditorTab::signal_color_base(SignalType type) {
	if (type == SignalType::Audio)
		return theme.blue;
	if (type == SignalType::Midi)
		return color(1, 0.9f,0.6f,0); // theme.green;
	if (type == SignalType::Beats)
		return theme.red;
	return theme.white;
}

color SignalEditorTab::signal_color(SignalType type, bool hover) {
	color c = signal_color_base(type);
	c = color::interpolate(c, theme.text, 0.2f);
//	c = color::interpolate(c, Gray, 0.6f);
	//c = color::interpolate(c, colors.background, 0.2f);
	if (hover)
		c = theme.hoverify(c);
	return c;
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

	if (graph())
		graph()->hover = HoverData();

	// TODO event routing...
	session->win->update_menu();

	update_module_positions();
}

void SignalEditorTab::popup_chain() {
	out_popup_chain();
	//menu_chain->open_popup((hui::Panel*)session->win.get());
}

void SignalEditorTab::popup_module() {
	out_popup_module();
	//menu_module->open_popup((hui::Panel*)session->win.get());
}

bool SignalEditorTab::on_key_down(int key) {
	if (key == hui::KEY_DELETE)
		on_module_delete();
	return true;
}

void SignalEditorTab::on_activate() {
}

void SignalEditorTab::on_delete() {
	if (chain->belongs_to_system) {
		session->e(_("not allowed to delete the main signal chain"));
		return;
	}
	hui::run_later(0.001f, [this] { chain->unregister(); });
}


void SignalEditorTab::on_add(ModuleCategory type) {
	auto names = session->plugin_manager->find_module_sub_types(type);
	if (names.num > 1) {
		ModuleSelectorDialog::choose(graph()->panel, session, type).then([this, type] (const string &name) {
			auto m = chain->add(type, name);
			m->module_x = graph()->m.x;
			m->module_y = graph()->m.y;
			update_module_positions();
		});
	} else {
		auto m = chain->add(type);
		m->module_x = graph()->m.x;
		m->module_y = graph()->m.y;
		update_module_positions();
	}
}

void SignalEditorTab::on_reset() {
	chain->reset(false);
}

/*void SignalEditorTabPanel::on_load() {
	if (hui::FileDialogOpen(win, "", "", "*.chain", "*.chain"))
		chain->load(hui::Filename);
}*/

void SignalEditorTab::on_save() {
	hui::file_dialog_save(graph()->panel->win, _("Save the signal chain"), session->storage->current_chain_directory, {"filter=*.chain", "showfilter=*.chain"}).then([this] (const Path &filename) {
		session->storage->current_chain_directory = filename.parent();
		chain->save(filename);
	});
}



void SignalEditorTab::on_module_delete() {
	for (auto m: sel_modules)
		chain->delete_module(m);
	select_module(nullptr);
}

void SignalEditorTab::select_module(Module *m, bool add) {
	if (!add)
		sel_modules.clear();
	if (m)
		sel_modules.add(m);
	out_module_selected(m);
	request_redraw();
}

void SignalEditorTab::update_module_positions() {
	// temporarily set a 1:1 coord system
	//pad->Node::update_geometry_recursive(pad->area);
	rect r = rect::EMPTY;
	if (modules.num > 0)
		r = modules[0]->area;

	for (auto *m: modules) {
		m->update_pos();
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

}
