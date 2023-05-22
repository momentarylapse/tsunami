/*
 * EffectsConsole.cpp
 *
 *  Created on: 22 May 2023
 *      Author: michi
 */

#include "EffectsConsole.h"

#include "../audioview/AudioView.h"
#include "../helper/Slider.h"
#include "../module/ModulePanel.h"
#include "../module/ConfigPanel.h"
#include "../helper/FxListEditor.h"
#include "../dialog/TemperamentDialog.h"
#include "../dialog/EditStringsDialog.h"
#include "../dialog/ModuleSelectorDialog.h"
#include "../../data/Track.h"
#include "../../data/base.h"
#include "../../module/synthesizer/Synthesizer.h"
#include "../../plugins/PluginManager.h"
#include "../../lib/base/sort.h"
#include "../../lib/base/iter.h"
#include "../../Session.h"

EffectsConsole::EffectsConsole(Session *session, SideBar *bar) :
	SideBarConsole(_("Effects"), "fx-console", session, bar)
{
	track = nullptr;
	from_resource("fx-editor");
	set_decimals(1);
}

void EffectsConsole::on_enter() {
	set_track(view->cur_track());
	view->subscribe(this, [this] { on_view_cur_track_change(); }, view->MESSAGE_CUR_TRACK_CHANGE);

}
void EffectsConsole::on_leave() {
	view->unsubscribe(this);
	set_track(nullptr);
}

void EffectsConsole::load_data() {
	//enable("name", track);

	if (track) {
	//	set_string("name", track->name);
	//	set_options("name", "placeholder=" + track->nice_name());
		//hide_control("edit_midi_fx", track->type != SignalType::MIDI);
		//hide_control("edit_synth", track->type != SignalType::MIDI);


	} else {
		hide_control("td_t_bars", true);
		set_string("tuning", "");
	}
}

void EffectsConsole::set_track(Track *t) {
	if (t == track)
		return;
	if (track)
		track->unsubscribe(this);
	fx_editor = nullptr;
	track = t;
	load_data();
	if (track) {
		fx_editor = new FxListEditor(track, this, "fx", true);
		track->subscribe(this, [this] { set_track(nullptr); }, track->MESSAGE_DELETE);
		track->subscribe(this, [this] { on_update(); }, track->MESSAGE_ANY);
	}
}

void EffectsConsole::on_view_cur_track_change() {
	set_track(view->cur_track());
}

void EffectsConsole::on_update() {
	load_data();
}
