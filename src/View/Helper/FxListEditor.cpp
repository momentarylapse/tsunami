/*
 * FxListEditor.cpp
 *
 *  Created on: May 4, 2021
 *      Author: michi
 */

#include "FxListEditor.h"
#include "../Helper/ModulePanel.h"
#include "../AudioView.h"
#include "../Graph/AudioViewTrack.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Module/Audio/AudioEffect.h"
#include "../../Module/Midi/MidiEffect.h"
#include "../../Module/ConfigPanel.h"
#include "../../Session.h"
#include "../../Plugins/PluginManager.h"
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


FxListEditor::FxListEditor(Track *t, hui::Panel *p, const string &_id, const string &_id_midi, bool hexpand) {
	panel = p;
	id_fx_list = _id;
	id_midi_fx_list = _id_midi;
	track = t;
	module_panel_mode = ModulePanel::Mode::DEFAULT;
	if (hexpand)
		module_panel_mode = ModulePanel::Mode::DEFAULT_H;
	assert(track);
	event_ids.add(panel->event_x(id_fx_list, "hui:select", [this] { on_fx_select(); }));
	event_ids.add(panel->event_x(id_fx_list, "hui:change", [this] { on_fx_edit(); }));
	event_ids.add(panel->event_x(id_fx_list, "hui:move", [this] { on_fx_move(); }));
	event_ids.add(panel->event_x(id_fx_list, "hui:right-button-down", [this] { on_fx_right_click(); }));
	event_ids.add(panel->event("add-fx", [this] { on_add_fx(); }));
	event_ids.add(panel->event("fx-add", [this] { on_add_fx(); }));
	event_ids.add(panel->event("fx-delete", [this] { on_fx_delete(); }));
	event_ids.add(panel->event("fx-enabled", [this] { on_fx_enabled(); }));
	event_ids.add(panel->event("fx-copy-from-track", [this] { on_fx_copy_from_track(); }));
	event_ids.add(panel->event_x(id_midi_fx_list, "hui:select", [this] { on_midi_fx_select(); }));
	event_ids.add(panel->event_x(id_midi_fx_list, "hui:change", [this] { on_midi_fx_edit(); }));
	event_ids.add(panel->event_x(id_midi_fx_list, "hui:move", [this] { on_midi_fx_move(); }));
	event_ids.add(panel->event("add-midi-fx", [this] { on_add_midi_fx(); }));


	auto *song = track->song;
	song->subscribe(this, [this] { update(); }, song->MESSAGE_ENABLE_FX);
	track->subscribe(this, [this] { update(); }, track->MESSAGE_ANY);

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

	config_panel = nullptr;

	selected_module = m;

	string config_grid_id = "grid-config";

	if (selected_module) {
		config_panel = new ModulePanel(m, panel, (ModulePanel::Mode)module_panel_mode);

		if (m->module_category == ModuleCategory::AUDIO_EFFECT) {
			AudioEffect *fx = (AudioEffect*)m;
			config_panel->set_func_enable([this, fx](bool enabled) {
				track->enable_effect(fx, enabled);
			});
			config_panel->set_func_delete([this, fx] {
				track->delete_effect(fx);
			});
		} else { // MIDI_EFFECT
			MidiEffect *fx = (MidiEffect*)m;
			config_panel->set_func_enable([this, fx](bool enabled) {
				track->enable_midi_effect(fx, enabled);
			});
			config_panel->set_func_delete([this, fx] {
				track->delete_midi_effect(fx);
			});
		}
		panel->embed(config_panel.get(), config_grid_id, 0, 0);

		m->subscribe(this, [this] {
			select_module(nullptr);
		}, m->MESSAGE_DELETE);
	}

	update_fx_list_selection();
	panel->expand("config-revealer", m);
}

void FxListEditor::on_fx_select() {
	int n = panel->get_int("");
	if (n >= 0)
		select_module(track->fx[n].get());
	else
		select_module(nullptr);
}

void FxListEditor::on_fx_edit() {
	int n = hui::get_event()->row;
	if (n >= 0)
		track->enable_effect(track->fx[n].get(), panel->get_cell("", n, hui::get_event()->column)._bool());
}

void FxListEditor::on_fx_move() {
	int s = hui::get_event()->row;
	int t = hui::get_event()->row_target;
	track->move_effect(s, t);
}

void FxListEditor::on_fx_right_click() {
	menu_fx = hui::create_resource_menu("popup-menu-fx-list", panel);
	int n = hui::get_event()->row;
	menu_fx->enable("fx-delete", n >= 0);
	menu_fx->enable("fx-enabled", n >= 0);
	if (n >= 0)
		menu_fx->check("fx-enabled", track->fx[n]->enabled);
	menu_fx->open_popup(panel);
}

void FxListEditor::on_fx_delete() {
	int n = panel->get_int(id_fx_list);
	if (n >= 0) {
		auto t = track;
		hui::run_later(0.001f, [t,n] {
			t->delete_effect(t->fx[n].get());
		});
	}
}

void FxListEditor::on_fx_enabled() {
	int n = panel->get_int(id_fx_list);
	if (n >= 0)
		track->enable_effect(track->fx[n].get(), !track->fx[n]->enabled);
}

void FxListEditor::on_fx_copy_from_track() {
	auto dlg = new TrackSelectionDialog(panel->win, track->song);
	hui::fly(dlg, [this, dlg] {
		if (dlg->selected and dlg->selected != track) {
			track->song->begin_action_group("copy fx from track");
			foreachb (auto *fx, weak(track->fx))
				track->delete_effect(fx);
			for (auto *fx: weak(dlg->selected->fx))
				track->add_effect((AudioEffect*)fx->copy());
			track->song->end_action_group();
		}
	});
}

void FxListEditor::on_add_fx() {
	session()->plugin_manager->choose_module(panel->win, session(), ModuleCategory::AUDIO_EFFECT, [this] (const string &name) {
		if (name == "")
			return;
		auto *effect = CreateAudioEffect(session(), name);
		track->add_effect(effect);
	});
}

void FxListEditor::on_midi_fx_select() {
	int n = panel->get_int("");
	if (n >= 0)
		select_module(track->midi_fx[n].get());
	else
		select_module(nullptr);
}

void FxListEditor::on_midi_fx_edit() {
	int n = hui::get_event()->row;
	if (n >= 0)
		track->enable_midi_effect(track->midi_fx[n].get(), panel->get_cell("", n, hui::get_event()->column)._bool());
}

void FxListEditor::on_midi_fx_move() {
	int s = hui::get_event()->row;
	int t = hui::get_event()->row_target;
	track->move_midi_effect(s, t);
}

void FxListEditor::on_add_midi_fx() {
	session()->plugin_manager->choose_module(panel->win, session(), ModuleCategory::MIDI_EFFECT, [this] (const string &name) {
		if (name == "")
			return;
		auto *effect = CreateMidiEffect(session(), name);
		track->add_midi_effect(effect);
	});
}

void FxListEditor::update_fx_list_selection() {
	if (track) {
		panel->set_int(id_fx_list, weak(track->fx).find((AudioEffect*)selected_module));
		panel->set_int(id_midi_fx_list, weak(track->midi_fx).find((MidiEffect*)selected_module));
	}
}

void FxListEditor::update() {
	if (!track)
		return;

	// fx list
	panel->reset(id_fx_list);
	for (auto *fx: weak(track->fx))
		panel->add_string(id_fx_list, b2s(fx->enabled) + "\\" + fx->module_class);

	// midi fx list
	panel->reset(id_midi_fx_list);
	for (auto *fx: weak(track->midi_fx))
		panel->add_string(id_midi_fx_list, b2s(fx->enabled) + "\\" + fx->module_class);

	update_fx_list_selection();
}

