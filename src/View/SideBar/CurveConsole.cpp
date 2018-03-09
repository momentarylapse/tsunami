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
			addString("list", t.niceStr(song));

			for (auto &tt : targets)
				if (t.p == tt.p){
					sel.add(i);
					break;
				}
		}
		setSelection("list", sel);

		event("hui:close", std::bind(&CurveTargetDialog::destroy, this));
		event("cancel", std::bind(&CurveTargetDialog::destroy, this));
		event("ok", std::bind(&CurveTargetDialog::onOk, this));
		event("list", std::bind(&CurveTargetDialog::onOk, this));
	}

	Array<Curve::Target> all_targets;
	Array<Curve::Target> &targets;

	void onOk()
	{
		Array<int> sel = getSelection("list");
		targets.clear();
		for (int i: sel)
			targets.add(all_targets[i]);
		destroy();
	}
};

CurveConsole::CurveConsole(Session *session) :
	SideBarConsole(_("Curves"), session)
{

	fromResource("curve_console");

	id_list = "curves";

	event("add", std::bind(&CurveConsole::onAdd, this));
	event("delete", std::bind(&CurveConsole::onDelete, this));
	event("target", std::bind(&CurveConsole::onTarget, this));
	eventX(id_list, "hui:select", std::bind(&CurveConsole::onListSelect, this));
	eventX(id_list, "hui:change", std::bind(&CurveConsole::onListEdit, this));
	event("edit_song", std::bind(&CurveConsole::onEditSong, this));
	event("edit_track", std::bind(&CurveConsole::onEditTrack, this));
	event("edit_fx", std::bind(&CurveConsole::onEditFx, this));

	song->subscribe(this, std::bind(&CurveConsole::onUpdate, this), song->MESSAGE_NEW);
	song->subscribe(this, std::bind(&CurveConsole::onUpdate, this), song->MESSAGE_ADD_CURVE);
	song->subscribe(this, std::bind(&CurveConsole::onUpdate, this), song->MESSAGE_DELETE_CURVE);
	song->subscribe(this, std::bind(&CurveConsole::onUpdate, this), song->MESSAGE_EDIT_CURVE);
	view->subscribe(this, std::bind(&CurveConsole::onViewChange, this), view->MESSAGE_VIEW_CHANGE);
}

CurveConsole::~CurveConsole()
{
	song->unsubscribe(this);
	view->unsubscribe(this);
}


void CurveConsole::onViewChange()
{
	redraw("area");
}

void CurveConsole::onUpdate()
{
	updateList();
}

void CurveConsole::onEnter()
{
	view->setMode(view->mode_curve);
}

void CurveConsole::onLeave()
{
	view->setMode(view->mode_default);
}

void CurveConsole::updateList()
{
	reset(id_list);
	foreachi(Curve *c, song->curves, i){
		addString(id_list, c->name + format("\\%.3f\\%.3f\\", c->min, c->max) + c->getTargets(song));
		if (c == curve())
			setInt(id_list, i);
	}
}

void CurveConsole::onAdd()
{
	Array<Curve::Target> targets;
	CurveTargetDialog *dlg = new CurveTargetDialog(this, song, targets);
	dlg->run();
	delete(dlg);
	if (targets.num > 0){
		Curve *c = song->addCurve("new", targets);
		view->mode_curve->setCurve(c);
		updateList();
	}
}

void CurveConsole::onDelete()
{
	int n = getInt(id_list);
	if (n >= 0){
		song->deleteCurve(song->curves[n]);
		view->mode_curve->setCurve(NULL);
	}
}

void CurveConsole::onTarget()
{
	if (!curve())
		return;
	Array<Curve::Target> targets = curve()->targets;
	CurveTargetDialog *dlg = new CurveTargetDialog(this, song, targets);
	dlg->run();
	//if (dlg->id == "ok")
		song->curveSetTargets(curve(), targets);

	delete(dlg);
	updateList();
}

void CurveConsole::onListSelect()
{
	view->mode_curve->setCurve(NULL);
	int n = getInt(id_list);
	if (n >= 0){
		view->mode_curve->setCurve(song->curves[n]);
	}else{
		view->mode_curve->setCurve(NULL);
	}
	redraw("area");
}

void CurveConsole::onListEdit()
{
	int n = hui::GetEvent()->row;
	int col = hui::GetEvent()->column;
	if (n >= 0){
		string name = song->curves[n]->name;
		float min = song->curves[n]->min;
		float max = song->curves[n]->max;
		if (col == 0)
			name = getCell(id_list, n, col);
		else if (col == 1)
			min = getCell(id_list, n, col)._float();
		else if (col == 2)
			max = getCell(id_list, n, col)._float();
		song->editCurve(song->curves[n], name, min, max);
	}
	redraw("area");
}

void CurveConsole::onEditSong()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void CurveConsole::onEditTrack()
{
	bar()->open(SideBar::TRACK_CONSOLE);
}

void CurveConsole::onEditFx()
{
	bar()->open(SideBar::FX_CONSOLE);
}

Curve* CurveConsole::curve()
{
	return view->mode_curve->curve;
}

