#include "BarsEditorConsole.h"
#include "../audioview/AudioView.h"
#include "../audioview/graph/AudioViewTrack.h"
#include "../mode/ViewModeEditBars.h"
#include "../../module/Module.h"
#include "../../plugins/PluginManager.h"
#include "../../TsunamiWindow.h"
#include "../../Session.h"
#include "../../EditModes.h"



BarsEditorConsole::BarsEditorConsole(Session *session, SideBar *bar) :
	SideBarConsole(_("Bars editor"), "bars-editor", session, bar)
{
	from_resource("bars-editor");

	event("mode-select", [this] {
		on_edit_mode((int)ViewModeEditBars::EditMode::SELECT);
	});
	event("mode-rubber", [this] {
		on_edit_mode((int)ViewModeEditBars::EditMode::RUBBER);
	});
	/*event("stretch-apply", [this] {
		view->mode_edit_bars->apply_stretch();
	});
	event("compensate-pitch", [this] {
		view->mode_edit_bars->flag_pitch_compensate = is_checked("");
	});*/
	event("action-edit-speed", [this] {
		on_action_edit_speed();
	});
	event("action-replace", [this] {
		on_action_replace();
	});
	event("edit_track", [session] {
		session->set_mode(EditMode::DefaultTrack);
	});
	event("edit_song", [session] {
		session->set_mode(EditMode::DefaultSong);
	});

	view->mode_edit_bars->subscribe(this, [this] {
		update();
	}, view->mode_edit_bars->MESSAGE_ANY);
	update();

	view->subscribe(this, [this]{ update(); }, view->MESSAGE_CUR_TRACK_CHANGE);
}

BarsEditorConsole::~BarsEditorConsole() {
	view->mode_edit_bars->unsubscribe(this);
}

void BarsEditorConsole::on_layer_delete() {
}
void BarsEditorConsole::on_view_cur_layer_change() {
}

void BarsEditorConsole::on_edit_mode(int m) {
	auto mode = (ViewModeEditBars::EditMode)m;
	//expand("revealer-stretch", mode == ViewModeEditBars::EditMode::RUBBER);
	view->mode_edit_bars->set_edit_mode(mode);
}

void BarsEditorConsole::on_action_edit_speed() {
	view->session->win->on_edit_bars_speed();
}

void BarsEditorConsole::on_action_replace() {
	view->session->win->on_replace_bars();
}

void BarsEditorConsole::clear() {
}
void BarsEditorConsole::set_layer(TrackLayer *t) {
}

void BarsEditorConsole::update() {
	check("mode-select", view->mode_edit_bars->edit_mode == ViewModeEditBars::EditMode::SELECT);
	check("mode-rubber", view->mode_edit_bars->edit_mode == ViewModeEditBars::EditMode::RUBBER);
}

