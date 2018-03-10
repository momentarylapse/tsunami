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

CpuDisplay::CpuDisplay(hui::Panel* _panel, const string& _id, Session *session)
{
	panel = _panel;
	id = _id;
	perf_mon = session->perf_mon;
	view = session->view;

	if (!hui::Config.getBool("CpuDisplay", false))
		panel->hideControl(id, true);

	panel->eventXP(id, "hui:draw", std::bind(&CpuDisplay::onDraw, this, std::placeholders::_1));
	//panel->eventX(id, "hui:left-button-down", std::bind(&CpuDisplay::onLeftButtonDown, this));

	perf_mon->subscribe(this, std::bind(&CpuDisplay::onUpdate, this));
}

CpuDisplay::~CpuDisplay()
{
	perf_mon->unsubscribe(this);
}

color type_color(int t)
{
	if (t == CpuDisplay::TYPE_VIEW)
		return color(1, 0.3f, 0.7f, 0.4f);
	if (t == CpuDisplay::TYPE_OUT)
		return color(1, 0.4f, 0.3f, 0.8f);
	if (t == CpuDisplay::TYPE_SUCK)
		return color(1, 0.8f, 0.3f, 0.3f);
	return Black;
}

string type_name(int t)
{
	if (t == CpuDisplay::TYPE_VIEW)
		return "view";
	if (t == CpuDisplay::TYPE_OUT)
		return "out";
	if (t == CpuDisplay::TYPE_SUCK)
		return "suck";
	return "?";
}

void CpuDisplay::onDraw(Painter* p)
{
	int w = p->width;
	int h = p->height;

	p->setColor(view->colors.background);
	p->drawRect(2, 2, w-4, h-4);

	for (int t=0; t<NUM_TYPES; t++){
		p->setColor(type_color(t));
		for (int j=1; j<cpu[t].num; j++){
			float x0 = w - 2 - (cpu[t].num - j - 1) * 2;
			float x1 = w - 2 - (cpu[t].num - j    ) * 2;
			float y0 = 2 + (h - 4) * (1 - cpu[t][j-1]);
			float y1 = 2 + (h - 4) * (1 - cpu[t][j]);
			if (x1 >= 2)
				p->drawLine(x0, y0, x1, y1);
		}
		if (cpu[t].num > 0){
			p->setFontSize(8);
			p->drawStr(20 + t * 30, h / 2-10, format("%.0f%%", cpu[t].back() * 100));
			p->drawStr(20 + t * 30, h / 2, format("%.0fms", avg[t].back() * 1000));
		}
	}
}

void CpuDisplay::onUpdate()
{
	float c[NUM_TYPES], a[NUM_TYPES];
	for (int t=0; t<NUM_TYPES; t++)
		c[t] = a[t] = 0;

	auto infos = perf_mon->get_info();
	for (auto &i: infos)
		for (int t=0; t<NUM_TYPES; t++)
			if (i.name == type_name(t)){
				c[t] += i.cpu;
				a[t] += i.avg;
			}

	for (int t=0; t<NUM_TYPES; t++){
		cpu[t].add(c[t]);
		avg[t].add(a[t]);
	}

	panel->redraw(id);
}
