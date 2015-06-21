/*
 * CurveConsole.cpp
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#include "CurveConsole.h"
#include "../AudioView.h"
#include "../../Data/AudioFile.h"
#include "../../Data/Curve.h"

class CurveTargetDialog : public HuiDialog
{
public:
	CurveTargetDialog(HuiPanel *parent, AudioFile *a, Array<Curve::Target> &t) :
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
		all_targets = Curve::Target::enumerate(a);

		Array<int> sel;

		foreachi(Curve::Target &t, all_targets, i){
			addString("list", t.niceStr(a));

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

CurveConsole::CurveConsole(AudioView *_view, AudioFile *_audio) :
	BottomBarConsole(_("Kurven")),
	Observer("CurveConsole")
{
	view = _view;
	audio = _audio;

	addGrid("", 0, 0, 2, 1, "root");
	setTarget("root", 0);
	addDrawingArea("!grabfocus", 0, 0, 0, 0, "area");
	addGrid("", 1, 0, 1, 2, "controller");
	setTarget("controller", 0);
	addListView("!noexpandx,format=TTTt,width=300\\name\\min\\max\\target", 0, 0, 0, 0, "list");
	addGrid("", 0, 1, 4, 1, "controller_buttons");
	setTarget("controller_buttons", 0);
	addButton("add", 0, 0, 0, 0, "add");
	addButton("delete", 1, 0, 0, 0, "delete");
	addButton("target", 2, 0, 0, 0, "target");

	event("add", this, &CurveConsole::onAdd);
	event("delete", this, &CurveConsole::onDelete);
	event("target", this, &CurveConsole::onTarget);
	eventX("list", "hui:select", this, &CurveConsole::onListSelect);
	eventX("list", "hui:change", this, &CurveConsole::onListEdit);
	eventX("area", "hui:key-down", this, &CurveConsole::onKeyDown);
	eventX("area", "hui:left-button-down", this, &CurveConsole::onLeftButtonDown);
	eventX("area", "hui:left-button-up", this, &CurveConsole::onLeftButtonUp);
	eventX("area", "hui:mouse-move", this, &CurveConsole::onMouseMove);
	eventX("area", "hui:draw", this, &CurveConsole::onDraw);

	curve = NULL;
	curve_rect = rect(0, 0, 0, 0);
	hover = selected = -1;

	subscribe(audio, audio->MESSAGE_NEW);
	subscribe(audio, audio->MESSAGE_ADD_CURVE);
	subscribe(audio, audio->MESSAGE_DELETE_CURVE);
	subscribe(view, view->MESSAGE_VIEW_CHANGE);
}

CurveConsole::~CurveConsole()
{
	unsubscribe(audio);
	unsubscribe(view);
}

void CurveConsole::onUpdate(Observable* o, const string &message)
{
	if (o == audio){
		updateList();
	}else if (o == view){
		redraw("area");
	}
}

void CurveConsole::updateList()
{
	reset("list");
	foreachi(Curve *c, audio->curves, i){
		addString("list", c->name + format("\\%.3f\\%.3f\\", c->min, c->max) + c->getTargets(audio));
		if (c == curve)
			setInt("list", i);
	}
}

void CurveConsole::onAdd()
{
	Curve *c = new Curve;
	c->name = "new";
	audio->curves.add(c);
	audio->notify(audio->MESSAGE_ADD_CURVE);
}

void CurveConsole::onDelete()
{
	int n = getInt("list");
	if (n >= 0){
		delete(audio->curves[n]);
		audio->curves.erase(n);
		curve = NULL;
	}
	audio->notify(audio->MESSAGE_DELETE_CURVE);
}

void CurveConsole::onTarget()
{
	if (!curve)
		return;
	CurveTargetDialog *dlg = new CurveTargetDialog(this, audio, curve->targets);
	dlg->run();
	updateList();
}

void CurveConsole::onListSelect()
{
	curve = NULL;
	int n = getInt("list");
	if (n >= 0)
		curve = audio->curves[n];
	redraw("area");
}

void CurveConsole::onListEdit()
{
	int n = HuiGetEvent()->row;
	int col = HuiGetEvent()->column;
	if (n >= 0){
		if (col == 0)
			audio->curves[n]->name = getCell("list", n, col);
		else if (col == 1)
			audio->curves[n]->min = getCell("list", n, col)._float();
		else if (col == 2)
			audio->curves[n]->max = getCell("list", n, col)._float();
	}
	redraw("area");
}

void CurveConsole::onKeyDown()
{
	if ((curve) && (selected >= 0))
		if (HuiGetEvent()->key_code == KEY_DELETE){
			curve->points.erase(selected);
			selected = hover = -1;
			redraw("area");
		}
}

void CurveConsole::onLeftButtonDown()
{
	hover = getHover();
	selected = hover;

	if ((curve) && (selected < 0)){
		curve->add(screen2sample(HuiGetEvent()->mx), screen2value(HuiGetEvent()->my));
		redraw("area");
	}
}

void CurveConsole::onLeftButtonUp()
{
}

int CurveConsole::getHover()
{
	if (!curve)
		return -1;
	float mx = HuiGetEvent()->mx;
	float my = HuiGetEvent()->my;
	foreachi(Curve::Point &p, curve->points, i){
		float x = sample2screen(p.pos);
		float y = value2screen(p.value);
		if ((fabs(mx - x) < 10) && (fabs(my - y) < 10))
			return i;
	}
	return -1;
}

void CurveConsole::onMouseMove()
{
	float mx = HuiGetEvent()->mx;
	float my = HuiGetEvent()->my;
	if (HuiGetEvent()->lbut){
		if ((curve) && (selected >= 0)){
			curve->points[selected].pos = screen2sample(mx);
			curve->points[selected].value = clampf(screen2value(my), curve->min, curve->max);
		}

	}else{
		hover = getHover();
	}
	redraw("area");
}

void CurveConsole::onDraw()
{
	HuiPainter *c = beginDraw("area");
	float w = c->width;
	float h = c->height;
	float y0 = 10;
	float y1 = h - 10;
	curve_rect = rect(0, w, y0, y1);
	c->setLineWidth(1.0f);
	c->setColor(Black);
	c->drawLine(0, y0, w, y0);
	c->drawLine(0, y1, w, y1);
	if (curve){
		Array<complex> pp;
		for (int x=0; x<w; x+=5)
			pp.add(complex(x, value2screen(curve->get(screen2sample(x)))));
		c->drawLines(pp);
		foreachi(Curve::Point &p, curve->points, i){
			if (i == hover)
				c->setColor(Red);
			else if (i == selected)
				c->setColor(Blue);
			else
				c->setColor(Black);
			c->drawCircle(sample2screen(p.pos), value2screen(p.value), 3);
		}
	}
	c->end();
}

float CurveConsole::sample2screen(float pos)
{
	return view->cam.sample2screen(pos) * curve_rect.width() / view->drawing_rect.width();
}

float CurveConsole::screen2sample(float x)
{
	return view->cam.screen2sample(x * view->drawing_rect.width() / curve_rect.width());
}

float CurveConsole::value2screen(float value)
{
	if (!curve)
		return 0;
	return curve_rect.y2 - curve_rect.height() * (value - curve->min) / (curve->max - curve->min);
}

float CurveConsole::screen2value(float y)
{
	if (!curve)
		return 0;
	return curve->min + (curve_rect.y2 - y) / curve_rect.height() * (curve->max - curve->min);
}

