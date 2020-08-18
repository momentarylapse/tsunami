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
#include "../../Module/Module.h"
#include "../AudioView.h"

static const float UPDATE_DT = 2.0f;

CpuDisplay::CpuDisplay(hui::Panel* _panel, const string& _id, Session *session) {
	panel = _panel;
	id = _id;
	perf_mon = session->perf_mon;
	view = session->view;

	dlg = nullptr;

	if (!hui::Config.get_bool("CpuDisplay", false))
		panel->hide_control(id, true);

	panel->event_xp(id, "hui:draw", [=](Painter* p){ on_draw(p); });
	panel->event_x(id, "hui:left-button-down", [=]{ on_left_button_down(); });

	perf_mon->subscribe(this, [=]{ update(); });
}

CpuDisplay::~CpuDisplay() {
	perf_mon->unsubscribe(this);
	if (dlg)
		delete dlg;
}

color type_color(const string &t) {
	if (t == "view")
		return color(1, 0.1f, 0.9f, 0.2f);
	if (t == "peak")
		return color(1, 0.1f, 0.9f, 0.9f);
	if (t == "module")
		return color(1, 0.2f, 0.2f, 0.9f);
	if (t == "suck")
		return color(1, 0.9f, 0.1f, 0.1f);
	return AudioView::colors.text_soft1;
}

string channel_title(PerfChannelInfo &c) {
	if (c.name != "module")
		return c.name;

	auto *m = (Module*)c.p;
	if (m->module_subtype != "")
		return m->module_subtype;

	return m->type_to_name(m->module_type);
}

void CpuDisplay::on_draw(Painter* p) {
	int w = p->width;
	int h = p->height;
	bool large = (h > 50);

	p->set_color(view->colors.background);
	p->draw_rect(2, 2, w-4, h-4);
	p->set_line_width(large ? 2.0f : 1.0f);

	// graphs
	for (auto &c: channels) {
		p->set_color(type_color(c.name));
		for (int j=1; j<c.stats.num; j++) {
			float x0 = w - 2 - (c.stats.num - (j-1)) * 2;
			float x1 = w - 2 - (c.stats.num -  j   ) * 2;
			float y0 = 2 + (h - 4) * (1 - c.stats[j-1].cpu);
			float y1 = 2 + (h - 4) * (1 - c.stats[j].cpu);
			if (x1 >= 2)
				p->draw_line(x0, y0, x1, y1);
		}
	}
	
	
	if (large) {
		p->set_font_size(10);
		p->set_color(view->colors.text_soft1);
		p->draw_str(150, 10, "cpu");
		p->draw_str(210, 10, "avg");
		p->draw_str(270, 10, "freq");
		
		int t = 0;
		Array<int> indent;
		for (auto &c: channels) {
			if (c.stats.num > 0) {
				color col = color::interpolate(type_color(c.name), view->colors.text, 0.5f);
				p->set_color(col);
				if (c.stats.back().counter == 0)
					p->set_color(color::interpolate(col, view->colors.background, 0.7f));
				int dx = 0;
				if (c.parent >= 0) {
					for (int i=0; i<channels.num; i++)
						if (channels[i].id == c.parent)
							if (i < indent.num) {
								dx = indent[i] + 12;
							}
				}
				string name = channel_title(c);
				p->set_font_size(10);
				p->draw_str(20 + dx, 30  + t * 20, name);
				p->draw_str(160, 30  + t * 20, format("%.1f%%", c.stats.back().cpu * 100));
				p->draw_str(210, 30  + t * 20, format("%.2fms", c.stats.back().avg * 1000));
				p->draw_str(280, 30  + t * 20, format("%.1f", (float)c.stats.back().counter / UPDATE_DT));
				indent.add(dx);
			} else {
				indent.add(0);
			}
			t ++;
		}
	} else {
	
		p->set_font_size(7);
	
		int t = 0;
		for (auto &c: channels) {
			if (c.stats.num > 0) {
				color col = color::interpolate(type_color(c.name), view->colors.text, 0.5f);
				p->set_color(col);
				p->draw_str(20 + (t/2) * 30, h / 2-14 + (t%2)*12, format("%.0f%%", c.stats.back().cpu * 100));
				t ++;
			}
		}
	}
}

void CpuDisplay::on_left_button_down() {
	if (dlg) {
		dlg->show();
	} else {
		dlg = new hui::Dialog("cpu", 380, 260, panel->win, true);
		dlg->set_border_width(0);
		dlg->add_drawing_area("", 0, 0, "area");
		dlg->event_xp("area", "hui:draw", [=](Painter *p){ on_draw(p); });
		dlg->event("hui:close", [=]{ on_dialog_close(); });
		dlg->show();
	}
}

void CpuDisplay::on_dialog_close() {
	if (!dlg)
		return;
	dlg->hide();
}

void ch_sort(Array<PerfChannelInfo> &channels) {
	for (int i=0; i<channels.num; i++)
		for (int j=i+1; j<channels.num; j++) {
			if (channels[i].name.compare(channels[j].name) < 0) {
				channels.swap(i, j);
			} else if ((channels[i].name.compare(channels[j].name) == 0) and (channels[i].stats.back().cpu < channels[j].stats.back().cpu)) {
				channels.swap(i, j);
			}
		}
}

Array<PerfChannelInfo> ch_children(const Array<PerfChannelInfo> &channels, int parent) {
	Array<PerfChannelInfo> r;
	for (auto &c: channels)
		if (c.parent == parent)
			r.add(c);
	ch_sort(r);
	return r;
}

Array<PerfChannelInfo> ch_tree_sort(const Array<PerfChannelInfo> &ch) {
	auto r = ch_children(ch, -1);
	for (int i=0; i<r.num; i++) {
		auto x = ch_children(ch, r[i].id);
		foreachi (auto &c, x, j)
			r.insert(c, i+j+1);
	}
	return r;
}

void CpuDisplay::update() {
	channels = ch_tree_sort(perf_mon->get_info());

	panel->redraw(id);
	if (dlg)
		dlg->redraw("area");
}
