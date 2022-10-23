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
	event("mode-add-and-split", [this] {
		on_edit_mode((int)ViewModeEditBars::EditMode::ADD_AND_SPLIT);
	});
	event("mode-rubber", [this] {
		on_edit_mode((int)ViewModeEditBars::EditMode::RUBBER);
	});
	event("action-edit-speed", [this] {
		on_action_edit_speed();
	});
	event("action-replace", [this] {
		on_action_replace();
	});
	event("edit-song", [session] {
		session->set_mode(EditMode::DefaultSong);
	});
	event("edit-track", [session] {
		session->set_mode(EditMode::DefaultTrack);
	});
	event("edit-track-curves", [session] {
		session->set_mode(EditMode::Curves);
	});
}

void BarsEditorConsole::on_enter() {
	view->mode_edit_bars->subscribe(this, [this] {
		update();
	}, view->mode_edit_bars->MESSAGE_ANY);
	view->subscribe(this, [this] {
		update();
	}, view->MESSAGE_CUR_TRACK_CHANGE);
	update();
}

void BarsEditorConsole::on_leave() {
	view->mode_edit_bars->unsubscribe(this);
	view->unsubscribe(this);
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
	check("mode-add-and-split", view->mode_edit_bars->edit_mode == ViewModeEditBars::EditMode::ADD_AND_SPLIT);
	check("mode-rubber", view->mode_edit_bars->edit_mode == ViewModeEditBars::EditMode::RUBBER);
}

