/*
 * CpuDisplay.cpp
 *
 *  Created on: 10.03.2018
 *      Author: michi
 */

#include "CpuDisplay.h"
#include "Graph/SceneGraph.h"
#include "../../Stuff/PerformanceMonitor.h"
#include "../../lib/hui/hui.h"
#include "../../Session.h"
#include "../../Module/Module.h"
#include "../../TsunamiWindow.h"
#include "../AudioView.h"

static const float UPDATE_DT = 2.0f;

CpuDisplayAdapter::CpuDisplayAdapter(hui::Panel* _parent, const string& _id, CpuDisplay *_cpu_display) {
	parent = _parent;
	id = _id;
	cpu_display = _cpu_display;

	scene_graph = new scenegraph::SceneGraph([] {});
	scene_graph->add_child(cpu_display);
	cpu_display->align.horizontal = scenegraph::Node::AlignData::Mode::FILL;
	cpu_display->align.vertical = scenegraph::Node::AlignData::Mode::FILL;

	parent->event_xp(id, "hui:draw", [=](Painter* p){
		scene_graph->update_geometry_recursive(p->area());
		scene_graph->draw(p);
	});
	parent->event_x(id, "hui:left-button-down", [=]{ scene_graph->on_left_button_down(); });
}




CpuDisplay::CpuDisplay(Session *_session, hui::Callback _request_redraw) : scenegraph::NodeFree() {
	align.w = 120;
	align.h = 30;
	align.dx = -20;
	align.dy = -20;
	align.horizontal = AlignData::Mode::RIGHT;
	align.vertical = AlignData::Mode::BOTTOM;
	align.dz = 100;
	hidden = true;

	session = _session;
	perf_mon = session->perf_mon;
	view = session->view;
	request_redraw = _request_redraw;

	dlg = nullptr;

	if (hui::Config.get_bool("CpuDisplay", false))
		enable(true);
}

CpuDisplay::~CpuDisplay() {
	enable(false);
	if (dlg)
		delete dlg;
}

void CpuDisplay::enable(bool active) {
	perf_mon->unsubscribe(this);

	hidden = !active;
	request_redraw();

	if (active)
		perf_mon->subscribe(this, [=]{ update(); });
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

	auto *m = reinterpret_cast<Module*>(c.p);
	if (m->module_subtype != "")
		return m->module_subtype;

	return m->type_to_name(m->module_type);
}

void CpuDisplay::draw(Painter* p) {
	float x00 = area.x1;
	float y00 = area.y1;
	int w = area.width();
	int h = area.height();
	bool large = (h > 50);

	auto old_clip = p->clip();
	p->set_clip(area);

	p->set_color(view->colors.background);
	p->draw_rect(area);
	p->set_line_width(large ? 2.0f : 1.0f);

	// graphs
	for (auto &c: channels) {
		p->set_color(type_color(c.name));
		for (int j=1; j<c.stats.num; j++) {
			float x0 = area.x2 - (c.stats.num - (j-1)) * 2;
			float x1 = area.x2 - (c.stats.num -  j   ) * 2;
			float y0 = area.y1 + h * (1 - c.stats[j-1].cpu);
			float y1 = area.y1 + h * (1 - c.stats[j].cpu);
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

		p->set_fill(false);
		p->set_color(view->colors.text_soft3);
		p->draw_rect(area);
		p->set_fill(true);
	
		p->set_font_size(7);
	
		int t = 0;
		for (auto &c: channels) {
			if (c.stats.num > 0) {
				color col = color::interpolate(type_color(c.name), view->colors.text, 0.5f);
				p->set_color(col);
				p->draw_str(x00 + 7 + (t/2) * 30, y00 + h / 2-10 + (t%2)*12, format("%2.0f%%", c.stats.back().cpu * 100));
				t ++;
			}
		}
	}
	p->set_clip(old_clip);
}

class CpuDisplayDialog : public hui::Dialog {
public:
	CpuDisplayDialog(Session *session) : hui::Dialog("cpu", 380, 260, session->win.get()->win, true) {
		set_options("", "resizable,borderwidth=0");
		add_drawing_area("", 0, 0, "area");

		auto cpu_display = new CpuDisplay(session, [&]{ redraw("area"); });
		adapter = new CpuDisplayAdapter(this, "area", cpu_display);
		event("hui:close", [=]{ hide(); });
	}

	CpuDisplayAdapter *adapter;
};

bool CpuDisplay::on_left_button_down() {
	if (!dlg)
		dlg = new CpuDisplayDialog(session);
	dlg->show();
	return true;
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
	request_redraw();
}
