/*
 * CpuDisplay.cpp
 *
 *  Created on: 10.03.2018
 *      Author: michi
 */

#include "CpuDisplay.h"
#include "../../Stuff/PerformanceMonitor.h"
#include "../../lib/hui/hui.h"
#include "../../Session.h"
#include "../AudioView.h"

static const float UPDATE_DT = 2.0f;

CpuDisplay::CpuDisplay(hui::Panel* _panel, const string& _id, Session *session)
{
	panel = _panel;
	id = _id;
	perf_mon = session->perf_mon;
	view = session->view;

	dlg = nullptr;

	if (!hui::Config.get_bool("CpuDisplay", false))
		panel->hideControl(id, true);

	panel->eventXP(id, "hui:draw", std::bind(&CpuDisplay::on_draw, this, std::placeholders::_1));
	panel->eventX(id, "hui:left-button-down", std::bind(&CpuDisplay::on_left_button_down, this));

	perf_mon->subscribe(this, std::bind(&CpuDisplay::update, this));
}

CpuDisplay::~CpuDisplay()
{
	perf_mon->unsubscribe(this);
	if (dlg)
		delete dlg;
}

color type_color(int t)
{
	if (t == CpuDisplay::TYPE_VIEW)
		return color(1, 0.1f, 0.9f, 0.2f);
	if (t == CpuDisplay::TYPE_PEAK)
		return color(1, 0.1f, 0.9f, 0.9f);
	if (t == CpuDisplay::TYPE_OUT)
		return color(1, 0.2f, 0.2f, 0.9f);
	if (t == CpuDisplay::TYPE_SUCK)
		return color(1, 0.9f, 0.1f, 0.1f);
	return Black;
}

string type_name(int t)
{
	if (t == CpuDisplay::TYPE_VIEW)
		return "view";
	if (t == CpuDisplay::TYPE_PEAK)
		return "peak";
	if (t == CpuDisplay::TYPE_OUT)
		return "out";
	if (t == CpuDisplay::TYPE_SUCK)
		return "suck";
	return "?";
}

void CpuDisplay::on_draw(Painter* p)
{
	int w = p->width;
	int h = p->height;
	bool large = (h > 50);

	p->setColor(view->colors.background);
	p->drawRect(2, 2, w-4, h-4);
	p->setLineWidth(large ? 1.5f : 1.0f);

	if (large){
		p->setFontSize(10);
		p->setColor(view->colors.text_soft1);
		p->drawStr(68, 10, "cpu");
		p->drawStr(118, 10, "avg");
		p->drawStr(173, 10, "freq");
	}

	for (int t=0; t<NUM_TYPES; t++){
		p->setColor(type_color(t));
		for (int j=1; j<cpu[t].num; j++){
			float x0 = w - 2 - (cpu[t].num - (j-1)) * 2;
			float x1 = w - 2 - (cpu[t].num -  j   ) * 2;
			float y0 = 2 + (h - 4) * (1 - cpu[t][j-1]);
			float y1 = 2 + (h - 4) * (1 - cpu[t][j]);
			if (x1 >= 2)
				p->drawLine(x0, y0, x1, y1);
		}
		if (cpu[t].num > 0){
			p->setColor(ColorInterpolate(type_color(t), view->colors.text, 0.5f));
			if (large){
				p->setFontSize(10);
				p->drawStr(20, 30  + t * 20, type_name(t));
				p->drawStr(68, 30  + t * 20, format("%.0f%%", cpu[t].back() * 100));
				p->drawStr(118, 30  + t * 20, format("%.0fms", avg[t].back() * 1000));
				p->drawStr(173, 30  + t * 20, format("%.1f", (float)count[t].back() / UPDATE_DT));

			}else{
				p->setFontSize(7);
				p->drawStr(20 + (t/2) * 30, h / 2-14 + (t%2)*12, format("%.0f%%", cpu[t].back() * 100));
			}
		}
	}
}

void CpuDisplay::on_left_button_down()
{
	if (dlg){
		dlg->show();

	}else{
		dlg = new hui::Dialog("cpu", 250, 180, panel->win, true);
		dlg->setBorderWidth(0);
		dlg->addDrawingArea("", 0, 0, "area");
		dlg->eventXP("area", "hui:draw", std::bind(&CpuDisplay::on_draw, this, std::placeholders::_1));
		dlg->event("hui:close", std::bind(&CpuDisplay::on_dialog_close, this));
		dlg->show();
	}
}

void CpuDisplay::on_dialog_close()
{
	if (!dlg)
		return;
	dlg->hide();
}

void CpuDisplay::update()
{
	float c[NUM_TYPES], a[NUM_TYPES];
	int cc[NUM_TYPES];
	for (int t=0; t<NUM_TYPES; t++){
		c[t] = a[t] = 0;
		cc[t] = 0;
	}

	auto infos = perf_mon->get_info();
	for (auto &i: infos)
		for (int t=0; t<NUM_TYPES; t++)
			if (i.name == type_name(t)){
				c[t] += i.cpu;
				a[t] += i.avg;
				cc[t] += i.counter;
			}

	for (int t=0; t<NUM_TYPES; t++){
		cpu[t].add(c[t]);
		avg[t].add(a[t]);
		count[t].add(cc[t]);
	}

	panel->redraw(id);
	if (dlg)
		dlg->redraw("area");
}
