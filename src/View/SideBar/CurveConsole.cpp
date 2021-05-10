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

/*
class CurveTargetDialog : public hui::Dialog {
public:
	CurveTargetDialog(hui::Panel *parent, Track *track, Array<Curve::Target> &t) :
		hui::Dialog(("curve-target-dialog"), parent->win),
		targets(t)
	{
		all_targets = Curve::Target::enumerate_track(track, "", "");

		Array<int> sel;

		foreachi(auto &t, all_targets, i) {
			add_string("list", t.nice_str(track->song));

			for (auto &tt : targets)
				if (t.p == tt.p){
					sel.add(i);
					break;
				}
		}
		set_selection("list", sel);

		event("hui:close", [=]{ request_destroy(); });
		event("cancel", [=]{ request_destroy(); });
		event("ok", [=]{ on_ok(); });
		event("list", [=]{ on_ok(); });
	}

	Array<Curve::Target> all_targets;
	Array<Curve::Target> &targets;

	void on_ok() {
		auto sel = get_selection("list");
		targets.clear();
		for (int i: sel)
			targets.add(all_targets[i]);
		request_destroy();
	}
};*/

CurveConsole::CurveConsole(Session *session) :
	SideBarConsole(_("Curves"), session)
{

	from_resource("curve_console");

	id_list = "curves";

	event("add", [=]{ on_add(); });
	event("delete", [=]{ on_delete(); });
	event("target", [=]{ on_target(); });
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
	view->mode_curve->set_curve(nullptr);
	update_list();
	auto t = track();
	song->subscribe(this, [=]{ on_update(); }, song->MESSAGE_NEW);
	t->subscribe(this, [=]{ on_update(); }, t->MESSAGE_ADD_CURVE);
	t->subscribe(this, [=]{ on_update(); }, t->MESSAGE_DELETE_CURVE);
	t->subscribe(this, [=]{ on_update(); }, t->MESSAGE_EDIT_CURVE);
	view->subscribe(this, [=]{ view->mode_curve->set_curve(nullptr); update_list(); }, view->MESSAGE_CUR_LAYER_CHANGE);
}

void CurveConsole::on_leave() {
	song->unsubscribe(this);
	track()->unsubscribe(this);
	view->unsubscribe(this);
	view->mode_curve->set_curve(nullptr);
}

void CurveConsole::update_list() {
	reset(id_list);
	curves.clear();
	for (auto *c: weak(track()->curves))
		curves.add(c);

	foreachi(Curve *c, curves, i) {
		add_string(id_list, c->get_target(track()) + format("\\%.3f\\%.3f", c->min, c->max));
		if (c == curve())
			set_int(id_list, i);
	}
}

void CurveConsole::on_add() {
	/*Array<Curve::Target> targets;
	auto dlg = ownify(new CurveTargetDialog(this, view->cur_track(), targets));
	dlg->run();
	if (targets.num > 0) {
		Curve *c = song->add_curve("new", targets);
		view->mode_curve->set_curve(c);
		update_list(); // for selection update...
	}*/
}

void CurveConsole::on_delete() {
	int n = get_int(id_list);
	if (n >= 0) {
		track()->delete_curve(curves[n]);
		view->mode_curve->set_curve(nullptr);
	}
}

void CurveConsole::on_target() {
	/*if (!curve())
		return;
	auto targets = curve()->target;
	auto dlg = ownify(new CurveTargetDialog(this, view->cur_track(), targets));
	dlg->run();
	//if (dlg->id == "ok")
		song->curve_set_targets(curve(), targets);

	update_list();*/
}

void CurveConsole::on_list_select() {
	view->mode_curve->set_curve(nullptr);
	int n = get_int(id_list);
	if (n >= 0) {
		view->mode_curve->set_curve(curves[n]);
	} else {
		view->mode_curve->set_curve(nullptr);
	}
	view->force_redraw();
}

void CurveConsole::on_list_edit() {
	int n = hui::GetEvent()->row;
	int col = hui::GetEvent()->column;
	if (n >= 0) {
		string name = curves[n]->name;
		float min = curves[n]->min;
		float max = curves[n]->max;
		if (col == 1)
			min = get_cell(id_list, n, col)._float();
		else if (col == 2)
			max = get_cell(id_list, n, col)._float();
		track()->edit_curve(curves[n], name, min, max);
	}
	view->force_redraw();
}

Curve* CurveConsole::curve() {
	return view->mode_curve->curve;
}

