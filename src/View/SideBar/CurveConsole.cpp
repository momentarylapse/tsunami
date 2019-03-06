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
#include "../../Data/Curve.h"
#include "../../Session.h"

class CurveTargetDialog : public hui::Window
{
public:
	CurveTargetDialog(hui::Panel *parent, Song *song, Array<Curve::Target> &t) :
		hui::Window(("curve-target-dialog"), parent->win),
		targets(t)
	{
		all_targets = Curve::Target::enumerate(song);

		Array<int> sel;

		foreachi(Curve::Target &t, all_targets, i){
			add_string("list", t.niceStr(song));

			for (auto &tt : targets)
				if (t.p == tt.p){
					sel.add(i);
					break;
				}
		}
		set_selection("list", sel);

		event("hui:close", std::bind(&CurveTargetDialog::destroy, this));
		event("cancel", std::bind(&CurveTargetDialog::destroy, this));
		event("ok", std::bind(&CurveTargetDialog::onOk, this));
		event("list", std::bind(&CurveTargetDialog::onOk, this));
	}

	Array<Curve::Target> all_targets;
	Array<Curve::Target> &targets;

	void onOk()
	{
		Array<int> sel = get_selection("list");
		targets.clear();
		for (int i: sel)
			targets.add(all_targets[i]);
		destroy();
	}
};

CurveConsole::CurveConsole(Session *session) :
	SideBarConsole(_("Curves"), session)
{

	from_resource("curve_console");

	id_list = "curves";

	event("add", std::bind(&CurveConsole::on_add, this));
	event("delete", std::bind(&CurveConsole::on_delete, this));
	event("target", std::bind(&CurveConsole::on_target, this));
	event_x(id_list, "hui:select", std::bind(&CurveConsole::on_list_select, this));
	event_x(id_list, "hui:change", std::bind(&CurveConsole::on_list_edit, this));
	event("edit_song", std::bind(&CurveConsole::on_edit_song, this));
	event("edit_track", std::bind(&CurveConsole::on_edit_track, this));
	event("edit_fx", std::bind(&CurveConsole::on_edit_fx, this));

	song->subscribe(this, std::bind(&CurveConsole::on_update, this), song->MESSAGE_NEW);
	song->subscribe(this, std::bind(&CurveConsole::on_update, this), song->MESSAGE_ADD_CURVE);
	song->subscribe(this, std::bind(&CurveConsole::on_update, this), song->MESSAGE_DELETE_CURVE);
	song->subscribe(this, std::bind(&CurveConsole::on_update, this), song->MESSAGE_EDIT_CURVE);
	view->subscribe(this, std::bind(&CurveConsole::on_view_change, this), view->MESSAGE_VIEW_CHANGE);
}

CurveConsole::~CurveConsole()
{
	song->unsubscribe(this);
	view->unsubscribe(this);
}


void CurveConsole::on_view_change()
{
	view->force_redraw();
}

void CurveConsole::on_update()
{
	update_list();
}

void CurveConsole::on_enter()
{
}

void CurveConsole::on_leave()
{
}

void CurveConsole::update_list()
{
	reset(id_list);
	foreachi(Curve *c, song->curves, i){
		add_string(id_list, c->name + format("\\%.3f\\%.3f\\", c->min, c->max) + c->getTargets(song));
		if (c == curve())
			set_int(id_list, i);
	}
}

void CurveConsole::on_add()
{
	Array<Curve::Target> targets;
	CurveTargetDialog *dlg = new CurveTargetDialog(this, song, targets);
	dlg->run();
	delete(dlg);
	if (targets.num > 0){
		Curve *c = song->add_curve("new", targets);
		view->mode_curve->setCurve(c);
		update_list();
	}
}

void CurveConsole::on_delete()
{
	int n = get_int(id_list);
	if (n >= 0){
		song->delete_curve(song->curves[n]);
		view->mode_curve->setCurve(nullptr);
	}
}

void CurveConsole::on_target()
{
	if (!curve())
		return;
	Array<Curve::Target> targets = curve()->targets;
	CurveTargetDialog *dlg = new CurveTargetDialog(this, song, targets);
	dlg->run();
	//if (dlg->id == "ok")
		song->curve_set_targets(curve(), targets);

	delete(dlg);
	update_list();
}

void CurveConsole::on_list_select()
{
	view->mode_curve->setCurve(nullptr);
	int n = get_int(id_list);
	if (n >= 0){
		view->mode_curve->setCurve(song->curves[n]);
	}else{
		view->mode_curve->setCurve(nullptr);
	}
	view->force_redraw();
}

void CurveConsole::on_list_edit()
{
	int n = hui::GetEvent()->row;
	int col = hui::GetEvent()->column;
	if (n >= 0){
		string name = song->curves[n]->name;
		float min = song->curves[n]->min;
		float max = song->curves[n]->max;
		if (col == 0)
			name = get_cell(id_list, n, col);
		else if (col == 1)
			min = get_cell(id_list, n, col)._float();
		else if (col == 2)
			max = get_cell(id_list, n, col)._float();
		song->edit_curve(song->curves[n], name, min, max);
	}
	view->force_redraw();
}

void CurveConsole::on_edit_song()
{
	session->set_mode("default/song");
}

void CurveConsole::on_edit_track()
{
	session->set_mode("default/track");
}

void CurveConsole::on_edit_fx()
{
	session->set_mode("default/fx");
}

Curve* CurveConsole::curve()
{
	return view->mode_curve->curve;
}

