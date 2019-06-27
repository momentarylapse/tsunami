/*
 * SynthConsole.cpp
 *
 *  Created on: 13.04.2014
 *      Author: michi
 */

#include "SynthConsole.h"
#include "../AudioView.h"
#include "../Helper/ModulePanel.h"
#include "../Dialog/ConfigurableSelectorDialog.h"
#include "../Dialog/DetuneSynthesizerDialog.h"
#include "../../Data/Track.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../../Module/ConfigPanel.h"
#include "../../Plugins/PluginManager.h"
#include "../../Session.h"



SynthConsole::SynthConsole(Session *session) :
	SideBarConsole(_("Synthesizer"), session)
{
	id_inner = "grid";

	from_resource("synth_console");

	event("select", [=]{ on_select(); });
	event("detune", [=]{ on_detune(); });

	event("edit_song", [=]{ on_edit_song(); });
	event("edit_track", [=]{ on_edit_track(); });

	track = nullptr;
	panel = nullptr;

	view->subscribe(this, [=]{ on_view_cur_track_change(); }, view->MESSAGE_CUR_TRACK_CHANGE);
}

SynthConsole::~SynthConsole() {
	view->unsubscribe(this);
	clear();
}

void SynthConsole::on_select() {
	if (!track)
		return;
	string name = session->plugin_manager->choose_module(win, session, ModuleType::SYNTHESIZER, track->synth->module_subtype);
	if (name != "")
		track->set_synthesizer(CreateSynthesizer(session, name));
}

void SynthConsole::on_detune() {
	auto *dlg = new DetuneSynthesizerDialog(track->synth, track, view, win);
	dlg->show();
}

void SynthConsole::on_edit_song() {
	session->set_mode("default/song");
}

void SynthConsole::on_edit_track() {
	session->set_mode("default/track");
}

void SynthConsole::clear() {
	if (track) {
		track->unsubscribe(this);
		if (track->synth and panel) {
			delete panel;
			panel = nullptr;
			remove_control("separator_0");
		}
	}
	track = nullptr;
}

void SynthConsole::set_track(Track *t) {
	clear();
	track = t;
	if (!track)
		return;

	track->subscribe(this, [=]{ on_track_delete(); }, track->MESSAGE_DELETE);
	track->subscribe(this, [=]{ on_track_change(); }, track->MESSAGE_REPLACE_SYNTHESIZER);

	if (track->synth) {
		panel = new ModulePanel(track->synth);
		panel->set_func_edit([=](const string &param){ track->edit_synthesizer(param); });
		embed(panel, id_inner, 0, 0);
		add_separator("!horizontal", 0, 1, "separator_0");
	}
}

void SynthConsole::on_track_delete() {
	clear();
}

void SynthConsole::on_track_change() {
	set_track(track);
}

void SynthConsole::on_view_cur_track_change() {
	set_track(view->cur_track());
}

