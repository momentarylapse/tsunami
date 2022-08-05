/*
 * CurveConsole.cpp
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#include "CurveConsole.h"
#include "../audioview/AudioView.h"
#include "../mode/ViewModeCurve.h"
#include "../mode/ViewModeDefault.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/Curve.h"
#include "../../Session.h"
#include "../../EditModes.h"

CurveConsole::CurveConsole(Session *session, SideBar *bar) :
	SideBarConsole(_("Curves"), "curve-console", session, bar)
{

	from_resource("curve_console");

	id_list = "curves";

	popup_menu = hui::create_resource_menu("popup-menu-curve", this);

	event("curve-delete", [this]{ on_delete(); });
	event("curve-linear", [this]{ on_type(CurveType::LINEAR); });
	event("curve-exponential", [this]{ on_type(CurveType::EXPONENTIAL); });
	event_x(id_list, "hui:select", [this]{ on_list_select(); });
	event_x(id_list, "hui:change", [this]{ on_list_edit(); });
	event_x(id_list, "hui:right-button-down", [this]{ on_list_right_click(); });
	event("edit_song", [session] {
		session->set_mode(EditMode::DefaultSong);
	});
	event("edit_track", [session] {
		session->set_mode(EditMode::DefaultTrack);
	});
	event("edit_midi", [session] {
		session->set_mode(EditMode::EditTrack);
	});

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
	enable("edit_synth", t->type == SignalType::MIDI);
	enable("edit_midi", t->type == SignalType::MIDI);
	song->subscribe(this, [this]{ on_update(); }, song->MESSAGE_NEW);
	t->subscribe(this, [this]{ on_update(); }, t->MESSAGE_ADD_CURVE);
	t->subscribe(this, [this]{ on_update(); }, t->MESSAGE_DELETE_CURVE);
	t->subscribe(this, [this]{ on_update(); }, t->MESSAGE_EDIT_CURVE);
	view->subscribe(this, [this] {
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
	float scroll = get_float(id_list);
	reset(id_list);

	auto t = track();

	targets = CurveTarget::enumerate_track(t);
	foreachi(auto &ct, targets, i) {
		if (auto c = track_find_curve(t, ct.id))
			add_string(id_list, format("<b>%s</b>\\%.3f\\%.3f", ct.nice_str(t), c->min, c->max));
		else
			add_string(id_list, format("<i><span alpha=\"50%%\">%s</span></i>\\(0)\\(1)", ct.nice_str(t)));
		if (ct.id == view->mode_curve->target)
			set_int(id_list, i);
	}
	set_float(id_list, scroll);
}

void CurveConsole::on_delete() {
	if (auto c = curve()) {
		track()->delete_curve(c);
		view->mode_curve->set_curve_target("");
	}
}

void CurveConsole::on_type(CurveType type) {
	if (auto c = curve())
		track()->edit_curve(c, c->name, c->min, c->max, type);
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
	int n = hui::get_event()->row;
	int col = hui::get_event()->column;
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
	track()->edit_curve(c, name, min, max, c->type);
	view->force_redraw();
}

void CurveConsole::on_list_right_click() {
	popup_menu->enable("curve-delete", curve());
	popup_menu->enable("curve-linear", curve());
	popup_menu->enable("curve-exponential", curve());
	if (auto c = curve()) {
		popup_menu->check("curve-linear", c->type == CurveType::LINEAR);
		popup_menu->check("curve-exponential", c->type == CurveType::EXPONENTIAL);
	}
	popup_menu->open_popup(this);
}

Curve* CurveConsole::curve() {
	return view->mode_curve->_curve;
}
