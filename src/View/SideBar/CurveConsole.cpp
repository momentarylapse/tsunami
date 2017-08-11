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

CurveConsole::CurveConsole(AudioView *_view, Song *_song) :
	SideBarConsole(_("Curves")),
	Observer("CurveConsole")
{
	view = _view;
	song = _song;

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

	curve = NULL;

	subscribe(song, song->MESSAGE_NEW);
	subscribe(song, song->MESSAGE_ADD_CURVE);
	subscribe(song, song->MESSAGE_DELETE_CURVE);
	subscribe(view, view->MESSAGE_VIEW_CHANGE);
}

CurveConsole::~CurveConsole()
{
	unsubscribe(song);
	unsubscribe(view);
}

void CurveConsole::onUpdate(Observable* o, const string &message)
{
	if (o == song){
		updateList();
	}else if (o == view){
		redraw("area");
	}
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
		if (c == curve)
			setInt(id_list, i);
	}
}

void CurveConsole::onAdd()
{
	Curve *c = new Curve;
	c->name = "new";
	song->curves.add(c);
	song->notify(song->MESSAGE_ADD_CURVE);
}

void CurveConsole::onDelete()
{
	int n = getInt(id_list);
	if (n >= 0){
		delete(song->curves[n]);
		song->curves.erase(n);
		curve = NULL;
	}
	song->notify(song->MESSAGE_DELETE_CURVE);
}

void CurveConsole::onTarget()
{
	if (!curve)
		return;
	CurveTargetDialog *dlg = new CurveTargetDialog(this, song, curve->targets);
	dlg->run();
	delete(dlg);
	updateList();
}

void CurveConsole::onListSelect()
{
	curve = NULL;
	view->mode_curve->setCurve(NULL);
	int n = getInt(id_list);
	if (n >= 0){
		curve = song->curves[n];
		view->mode_curve->setCurve(curve);
	}
	redraw("area");
}

void CurveConsole::onListEdit()
{
	int n = hui::GetEvent()->row;
	int col = hui::GetEvent()->column;
	if (n >= 0){
		if (col == 0)
			song->curves[n]->name = getCell(id_list, n, col);
		else if (col == 1)
			song->curves[n]->min = getCell(id_list, n, col)._float();
		else if (col == 2)
			song->curves[n]->max = getCell(id_list, n, col)._float();
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

