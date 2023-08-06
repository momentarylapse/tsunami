/*
 * FxListEditor.cpp
 *
 *  Created on: May 4, 2021
 *      Author: michi
 */

#include "FxListEditor.h"
#include "../dialog/ModuleSelectorDialog.h"
#include "../module/ModulePanel.h"
#include "../module/ConfigPanel.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../module/audio/AudioEffect.h"
#include "../../module/midi/MidiEffect.h"
#include "../../Session.h"
#include "../../plugins/PluginManager.h"
#include "../../lib/hui/hui.h"
#include <cassert>


class TrackSelectionDialog : public hui::Dialog {
public:
	Song *song;
	Track *selected = nullptr;
	TrackSelectionDialog(hui::Window *parent, Song *_song) : hui::Dialog("track-selector", parent) {
		song = _song;
		for (Track *t: weak(song->tracks))
			add_string("tracks", t->nice_name());
		event("tracks", [this] { on_select(); });
		event("ok", [this] { on_select(); });
		event("cancel", [this] { request_destroy(); });
	}
	void on_select() {
		int n = get_int("");
		if (n >= 0)
			selected = song->tracks[n].get();
		request_destroy();
	}
};


//todo set/unset track...


FxListEditor::FxListEditor(Track *t, hui::Panel *p, const string &_id, bool hexpand) {
	panel = p;
	id_list = _id;
	track = t;
	module_panel_mode = ConfigPanelMode::FIXED_WIDTH;
	if (hexpand)
		module_panel_mode = ConfigPanelMode::FIXED_HEIGHT;
	module_panel_mode = module_panel_mode | ConfigPanelMode::PROFILES | ConfigPanelMode::ENABLE | ConfigPanelMode::DELETE;
	module_panel_mode = module_panel_mode | ConfigPanelMode::WETNESS;
	assert(track);
	event_ids.add(panel->event_x(id_list, "hui:select", [this] { on_select(); }));
	event_ids.add(panel->event_x(id_list, "hui:change", [this] { on_edit(); }));
	event_ids.add(panel->event_x(id_list, "hui:move", [this] { on_move(); }));
	event_ids.add(panel->event_x(id_list, "hui:right-button-down", [this] { on_right_click(); }));
	event_ids.add(panel->event("add-fx", [this] { on_add(); }));
	event_ids.add(panel->event("fx-add", [this] { on_add(); }));
	event_ids.add(panel->event("fx-delete", [this] { on_delete(); }));
	event_ids.add(panel->event("fx-enabled", [this] { on_enabled(); }));
	event_ids.add(panel->event("fx-copy-from-track", [this] { on_copy_from_track(); }));

	dummy_module = CreateAudioEffect(session(), "");


	auto *song = track->song;
	song->out_enable_fx >> create_sink([this] { update(); });
	track->out_effect_list_changed >> create_sink([this] { update(); });
	track->out_changed >> create_sink([this] { update(); });

	update();
}

FxListEditor::~FxListEditor() {
	track->song->unsubscribe(this);
	track->unsubscribe(this);
	for (int e: event_ids)
		panel->remove_event_handler(e);
	select_module(nullptr);
}

Session *FxListEditor::session() const {
	if (!track)
		return nullptr;
	return track->song->session;
}

void FxListEditor::select_module(Module *m) {
	if (selected_module)
		selected_module->unsubscribe(this);
	if (config_panel)
		panel->unembed(config_panel.get());
	config_panel = nullptr;

	selected_module = m;

	string config_grid_id = "grid-config";

	if (selected_module) {
		config_panel = new ModulePanel(m, panel, module_panel_mode);

		if (m->module_category == ModuleCategory::AUDIO_EFFECT) {
			auto fx = reinterpret_cast<AudioEffect*>(m);
			config_panel->set_func_enable([this, fx](bool enabled) {
				track->enable_effect(fx, enabled);
			});
			config_panel->set_func_set_wetness([this, fx](float wetness) {
				track->set_effect_wetness(fx, wetness);
			});
			config_panel->set_func_delete([this, fx] {
				track->delete_effect(fx);
			});
		} else { // MIDI_EFFECT
			auto fx = reinterpret_cast<MidiEffect*>(m);
			config_panel->set_func_enable([this, fx](bool enabled) {
				track->enable_midi_effect(fx, enabled);
			});
			config_panel->set_func_delete([this, fx] {
				track->delete_midi_effect(fx);
			});
		}
		panel->embed(config_panel.get(), config_grid_id, 0, 0);

		m->out_death >> create_sink([this] {
			select_module(nullptr);
		});
	} else {
		// dummy panel to keep the size allocation for the animation
		config_panel = new ModulePanel(dummy_module.get(), panel, module_panel_mode);
		panel->embed(config_panel.get(), config_grid_id, 0, 0);
	}

	update_list_selection();
	panel->expand("config-revealer", m);
}

void FxListEditor::on_select() {
	int n = panel->get_int("");
	if (n >= 0)
		select_module(track->fx[n].get());
	else
		select_module(nullptr);
}

void FxListEditor::on_edit() {
	int n = hui::get_event()->row;
	if (n >= 0)
		track->enable_effect(track->fx[n].get(), panel->get_cell("", n, hui::get_event()->column)._bool());
}

void FxListEditor::on_move() {
	int s = hui::get_event()->row;
	int t = hui::get_event()->row_target;
	track->move_effect(s, t);
}

void FxListEditor::on_right_click() {
	menu = hui::create_resource_menu("popup-menu-fx-list", panel);
	int n = hui::get_event()->row;
	menu->enable("fx-delete", n >= 0);
	menu->enable("fx-enabled", n >= 0);
	if (n >= 0)
		menu->check("fx-enabled", track->fx[n]->enabled);
	menu->open_popup(panel);
}

void FxListEditor::on_delete() {
	int n = panel->get_int(id_list);
	if (n >= 0) {
		auto t = track;
		hui::run_later(0.001f, [t,n] {
			t->delete_effect(t->fx[n].get());
		});
	}
}

void FxListEditor::on_enabled() {
	int n = panel->get_int(id_list);
	if (n >= 0)
		track->enable_effect(track->fx[n].get(), !track->fx[n]->enabled);
}

void FxListEditor::on_copy_from_track() {
	auto dlg = new TrackSelectionDialog(panel->win, track->song);
	hui::fly(dlg, [this, dlg] {
		if (dlg->selected and dlg->selected != track) {
			track->song->begin_action_group("copy fx from track");
			foreachb (auto *fx, weak(track->fx))
				track->delete_effect(fx);
			for (auto *fx: weak(dlg->selected->fx))
				track->add_effect(reinterpret_cast<AudioEffect*>(fx->copy()));
			track->song->end_action_group();
		}
	});
}

void FxListEditor::on_add() {
	ModuleSelectorDialog::choose(panel->win, session(), ModuleCategory::AUDIO_EFFECT).on([this] (const string &name) {
		auto effect = CreateAudioEffect(session(), name);
		track->add_effect(effect);
	});
}

void FxListEditor::update_list_selection() {
	if (track)
		panel->set_int(id_list, weak(track->fx).find(reinterpret_cast<AudioEffect*>(selected_module)));
}

void FxListEditor::update() {
	if (!track)
		return;

	// fx list
	panel->reset(id_list);
	for (auto *fx: weak(track->fx)) {
		string pre, post;
		if (!fx->enabled) {
			pre = "<span alpha=\"50%\">";
			post = "</span>";
		}
		panel->add_string(id_list, format("%s\\%s%s\n<small>    %s, %.0f%%</small>%s", b2s(fx->enabled), pre, fx->module_class, (fx->enabled?"active":"disabled"), fx->wetness * 100, post));
	}

	update_list_selection();
}

