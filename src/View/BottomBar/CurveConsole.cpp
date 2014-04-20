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

CurveConsole::CurveConsole(AudioView *_view, AudioFile *_audio) :
	BottomBarConsole(_("Kurven")),
	Observer("CurveConsole")
{
	view = _view;
	audio = _audio;

	AddControlTable("", 0, 0, 2, 1, "root");
	SetTarget("root", 0);
	AddDrawingArea("", 0, 0, 0, 0, "area");
	AddControlTable("", 1, 0, 1, 2, "controller");
	SetTarget("controller", 0);
	AddListView("!noexpandx,width=250\\name\\min\\max\\target", 0, 0, 0, 0, "list");
	AddControlTable("", 0, 1, 3, 1, "controller_buttons");
	SetTarget("controller_buttons", 0);
	AddButton("add", 0, 0, 0, 0, "add");
	AddButton("target", 1, 0, 0, 0, "target");

	EventM("add", this, &CurveConsole::onAdd);
	EventM("target", this, &CurveConsole::onTarget);
	EventMX("list", "hui:select", this, &CurveConsole::onListSelect);
	EventMX("area", "hui:left-button-down", this, &CurveConsole::onLeftButtonDown);
	EventMX("area", "hui:left-button-up", this, &CurveConsole::onLeftButtonUp);
	EventMX("area", "hui:mouse-move", this, &CurveConsole::onMouseMove);
	EventMX("area", "hui:draw", this, &CurveConsole::onDraw);

	curve = NULL;
	curve_rect = rect(0, 0, 0, 0);
	hover = selected = -1;

	Subscribe(audio, audio->MESSAGE_NEW);
	Subscribe(audio, audio->MESSAGE_ADD_CURVE);
	Subscribe(audio, audio->MESSAGE_DELETE_CURVE);
	Subscribe(view, view->MESSAGE_VIEW_CHANGE);
}

CurveConsole::~CurveConsole()
{
	Unsubscribe(audio);
	Unsubscribe(view);
}

void CurveConsole::OnUpdate(Observable* o, const string &message)
{
	if (o == audio){
		updateList();
	}else if (o == view){
		Redraw("area");
	}
}

void CurveConsole::updateList()
{
	Reset("list");
	foreach(Curve *c, audio->curve){
		AddString("list", c->name + format("\\%.3f\\%.3f\\", c->min, c->max) + c->target);
	}
}

void CurveConsole::onAdd()
{
	Curve *c = new Curve;
	c->name = "new";
	c->target = "";
	c->min = 0;
	c->max = 1;
	c->type = c->TYPE_LINEAR;
	Curve::Point p;
	p.pos = 0;
	p.value = 0.2f;
	c->points.add(p);
	p.pos = 120000;
	p.value = 0.7f;
	c->points.add(p);
	audio->curve.add(c);
	audio->Notify(audio->MESSAGE_ADD_CURVE);
}

void CurveConsole::onTarget()
{
}

void CurveConsole::onListSelect()
{
	curve = NULL;
	int n = GetInt("list");
	if (n >= 0)
		curve = audio->curve[n];
	Redraw("area");
}

void CurveConsole::onLeftButtonDown()
{
	selected = hover;
}

void CurveConsole::onLeftButtonUp()
{
}

void CurveConsole::onMouseMove()
{
	hover = -1;
	float mx = HuiGetEvent()->mx;
	float my = HuiGetEvent()->my;
	if (HuiGetEvent()->lbut){
		if ((curve) && (selected >= 0)){
			curve->points[selected].pos = screen2sample(mx);
			curve->points[selected].value = clampf(screen2value(my), curve->min, curve->max);
		}

	}else{
		if (curve){
			foreachi(Curve::Point &p, curve->points, i){
				float x = sample2screen(p.pos);
				float y = value2screen(p.value);
				if ((fabs(mx - x) < 5) && (fabs(my - y) < 5))
					hover = i;
			}
		}
	}
	Redraw("area");
}

void CurveConsole::onDraw()
{
	HuiPainter *c = BeginDraw("area");
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
			c->setColor((i == hover) ? Red : Black);
			c->drawCircle(sample2screen(p.pos), value2screen(p.value), 3);
		}
	}
	c->end();
}

float CurveConsole::sample2screen(float pos)
{
	return view->sample2screen(pos) * curve_rect.width() / view->drawing_rect.width();
}

float CurveConsole::screen2sample(float x)
{
	return view->screen2sample(x * view->drawing_rect.width() / curve_rect.width());
}

float CurveConsole::value2screen(float value)
{
	if (!curve)
		return 0;
	return curve_rect.y1 + curve_rect.height() * (value - curve->min) / (curve->max - curve->min);
}

float CurveConsole::screen2value(float y)
{
	if (!curve)
		return 0;
	return curve->min + (y - curve_rect.y1) / curve_rect.height() * (curve->max - curve->min);
}

