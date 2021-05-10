/*
 * CurveConsole.cpp
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#include "CurveConsole.h"
#include "../AudioView.h"
#include "../Mode/ViewModeCurve.h"
#include "../Mode/ViewModeDefault.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/Curve.h"
#include "../../Session.h"
#include "../../EditModes.h"

CurveConsole::CurveConsole(Session *session) :
	SideBarConsole(_("Curves"), session)
{

	from_resource("curve_console");

	id_list = "curves";

	event("delete", [=]{ on_delete(); });
	event_x(id_list, "hui:select", [=]{ on_list_select(); });
	event_x(id_list, "hui:change", [=]{ on_list_edit(); });
	event("edit_song", [=]{ session->set_mode(EditMode::DefaultSong); });
	event("edit_track", [=]{ session->set_mode(EditMode::DefaultTrack); });
	event("edit_fx", [=]{ session->set_mode(EditMode::DefaultTrackFx); });

}

CurveConsole::~CurveConsole() {
	song->unsubscribe(this);
	view->unsubscribe(this);
}


void CurveConsole::on_update() {
	update_list();
}

Track *CurveConsole::track() {
	return view->cur_track();
}

void CurveConsole::on_enter() {
	view->mode_curve->set_curve_target("");
	update_list();
	auto t = track();
	song->subscribe(this, [=]{ on_update(); }, song->MESSAGE_NEW);
	t->subscribe(this, [=]{ on_update(); }, t->MESSAGE_ADD_CURVE);
	t->subscribe(this, [=]{ on_update(); }, t->MESSAGE_DELETE_CURVE);
	t->subscribe(this, [=]{ on_update(); }, t->MESSAGE_EDIT_CURVE);
	view->subscribe(this, [=] {
		view->mode_curve->set_curve_target("");
		update_list();
	}, view->MESSAGE_CUR_LAYER_CHANGE);
}

void CurveConsole::on_leave() {
	song->unsubscribe(this);
	track()->unsubscribe(this);
	view->unsubscribe(this);
	view->mode_curve->set_curve_target("");
}

Curve *track_find_curve(Track *t, const string &id) {
	for (auto *c: weak(t->curves)) {
		if (c->target.id == id)
			return c;
	}
	return nullptr;
}

void CurveConsole::update_list() {
	reset(id_list);

	auto t = track();

	targets = CurveTarget::enumerate_track(t);
	foreachi(auto &ct, targets, i) {
		if (auto c = track_find_curve(t, ct.id))
			add_string(id_list, format("<b>%s</b>\\%.3f\\%.3f", ct.nice_str(t), c->min, c->max));
		else
			add_string(id_list, format("<i>%s</i>\\(0)\\(1)", ct.nice_str(t)));
		if (ct.id == view->mode_curve->target)
			set_int(id_list, i);
	}
}

void CurveConsole::on_delete() {
	int n = get_int(id_list);
	if (n >= 0) {
		auto c = track_find_curve(track(), targets[n].id);
		if (c) {
			track()->delete_curve(c);
			view->mode_curve->set_curve_target("");
		}
	}
}

void CurveConsole::on_list_select() {
	int n = get_int(id_list);
	if (n >= 0) {
		view->mode_curve->set_curve_target(targets[n].id);
	} else {
		view->mode_curve->set_curve_target("");
	}
	view->force_redraw();
}

void CurveConsole::on_list_edit() {
	int n = hui::GetEvent()->row;
	int col = hui::GetEvent()->column;
	if (n < 0)
		return;
	auto c = track_find_curve(track(), targets[n].id);
	if (!c)
		return;
	string name = c->name;
	float min = c->min;
	float max = c->max;
	if (col == 1)
		min = get_cell(id_list, n, col)._float();
	else if (col == 2)
		max = get_cell(id_list, n, col)._float();
	track()->edit_curve(c, name, min, max);
	view->force_redraw();
}

Curve* CurveConsole::curve() {
	return view->mode_curve->_curve;
}

