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

class CurveTargetDialog : public HuiDialog
{
public:
	CurveTargetDialog(HuiPanel *parent, Song *song, Array<Curve::Target> &t) :
		HuiDialog(_("target"), 300, 400, parent->win, false),
		targets(t)
	{
		addGrid("", 0, 0, 1, 2, "grid");
		setTarget("grid", 0);
		addListView("!multiselection,nobar\\name", 0, 0, 0, 0, "list");
		addGrid("!buttonbar", 0, 1, 2, 1, "buttonbar");
		setTarget("buttonbar", 0);
		addButton(_("Abbrechen"), 0, 0, 0, 0, "cancel");
		addButton(_("Ok"), 1, 0, 0, 0, "ok");
		all_targets = Curve::Target::enumerate(song);

		Array<int> sel;

		foreachi(Curve::Target &t, all_targets, i){
			addString("list", t.niceStr(song));

			foreach(Curve::Target &tt, targets)
				if (t.p == tt.p){
					sel.add(i);
					break;
				}
		}
		setSelection("list", sel);

		event("hui:close", this, &CurveTargetDialog::onClose);
		event("cancel", this, &CurveTargetDialog::onClose);
		event("ok", this, &CurveTargetDialog::onOk);
	}

	Array<Curve::Target> all_targets;
	Array<Curve::Target> &targets;

	void onOk()
	{
		Array<int> sel = getSelection("list");
		targets.clear();
		foreach(int i, sel)
			targets.add(all_targets[i]);
		delete(this);
	}

	void onClose()
	{
		delete(this);
	}
};

CurveConsole::CurveConsole(AudioView *_view, Song *_song) :
	SideBarConsole(_("Kurven")),
	Observer("CurveConsole")
{
	view = _view;
	song = _song;

	fromResource("curve_console");

	id_list = "curves";

	event("add", this, &CurveConsole::onAdd);
	event("delete", this, &CurveConsole::onDelete);
	event("target", this, &CurveConsole::onTarget);
	eventX(id_list, "hui:select", this, &CurveConsole::onListSelect);
	eventX(id_list, "hui:change", this, &CurveConsole::onListEdit);

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
	int n = HuiGetEvent()->row;
	int col = HuiGetEvent()->column;
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

