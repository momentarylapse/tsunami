/*
 * CpuDisplay.cpp
 *
 *  Created on: 10.03.2018
 *      Author: michi
 */

#include "CpuDisplay.h"
#include "graph/SceneGraph.h"
#include "../MouseDelayPlanner.h"
#include "../audioview/graph/AudioViewLayer.h"
#include "../audioview/graph/AudioViewTrack.h"
#include "../ColorScheme.h"
#include "../painter/BasicGridPainter.h"
#include "../TsunamiWindow.h"
#include "../../stuff/PerformanceMonitor.h"
#include "../../lib/hui/hui.h"
#include "../../Session.h"
#include "../../module/Module.h"
#include "../../module/SignalChain.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"

static const float UPDATE_DT = 2.0f;



class CpuDisplayDialog : public hui::Dialog {
public:
	CpuDisplayDialog(Session *session) : hui::Dialog("cpu-display-dialog", session->win.get()->win) {
		// will be owned by graph
		auto cpu_display = new CpuDisplay(session);
		graph = scenegraph::SceneGraph::create_integrated(this, "area", cpu_display, "cpu");
		check("show-sleeping", cpu_display->show_sleeping);
		check("show-total", cpu_display->show_total);
		event("hui:close", [this] {
			hide();
		});
		event("show-sleeping", [this, cpu_display] {
			cpu_display->show_sleeping = is_checked("");
			cpu_display->update();
		});
		event("show-total", [this, cpu_display] {
			cpu_display->show_total = is_checked("");
			cpu_display->update();
		});
	}
	owned<scenegraph::SceneGraph> graph;
};


CpuDisplay::CpuDisplay(Session *_session) {
	align.w = 120;
	align.h = 30;
	align.dx = -20;
	align.dy = -20;
	align.horizontal = AlignData::Mode::RIGHT;
	align.vertical = AlignData::Mode::BOTTOM;
	align.dz = 100;
	clip = true;
	hidden = true;

	session = _session;
	perf_mon = session->perf_mon;
	view = session->view;
	show_sleeping = false;
	show_total = true;
	scroll_offset = 0;

	dlg = nullptr;

	set_perf_name("cpu");

	if (hui::config.get_bool("CpuDisplay", false))
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
		perf_mon->subscribe(this, [this] {
			update();
		}, perf_mon->MESSAGE_ANY);
}

color type_color(const string &t) {
	if ((t == "view") or (t == "graph") or (t == "node"))
		return color(1, 0.1f, 0.9f, 0.2f);
	if (t == "peakthread")
		return color(1, 0.9f, 0.2f, 0.2f);
	if (t == "SignalEditor")
		return color(1, 0.9f, 0.8f, 0.1f);
	if (t == "module")
		return color(1, 0.2f, 0.2f, 0.9f);
	if (t == "suck")
		return color(1, 0.9f, 0.1f, 0.1f);
	return theme.text_soft1;
}

string channel_title(PerfChannelInfo &c) {
	if (c.name == "module") {
		auto *m = reinterpret_cast<Module*>(c.p);
		if (m->module_category == ModuleCategory::SIGNAL_CHAIN) {
			auto *chain = reinterpret_cast<SignalChain*>(c.p);
			return chain->name;
		}
		if (m->module_class != "")
			return m->module_class;

		return m->category_to_str(m->module_category);
	} else if (c.name == "vtrack") {
		auto *t = reinterpret_cast<AudioViewTrack*>(c.p);
		if (t->track)
			return t->track->nice_name();
	} else if (c.name == "vlayer") {
		auto *l = reinterpret_cast<AudioViewLayer*>(c.p);
		if (l->layer)
			return l->layer->track->nice_name() + format(" v%d", l->layer->version_number()+1);
	}
	if (c.name.head(3) == "se:")
		return c.name.sub(3);
	return c.name;
}

void CpuDisplay::draw_background(Painter* p) {
	auto col_bg = large ? theme.background : theme.background_overlay;
	p->set_color(col_bg);
	p->draw_rect(area);

	if (!large)
		return;

	BasicGridPainter grid;
	GridColors gc = {col_bg, Black, theme.grid, Black};
	grid.set_context(area, gc);

	// horizontal time grid
	grid.plan_linear(-area.width(), 0, 20);
	grid.draw(p);

	// vertical percentage grid
	grid.horizontal = false;
	grid.plan_linear(0, 1, 20);
	grid.draw(p);

}

void CpuDisplay::draw_graphs(Painter* p) {
	float h = area.height();
	for (auto &c: channels) {
		if (c.parent >= 0)
			continue;
		p->set_color(type_color(c.name));
		for (int j=1; j<c.stats.num; j++) {
			float x0 = area.x2 - (c.stats.num - (j-1)) * 2;
			float x1 = area.x2 - (c.stats.num -  j   ) * 2;
			float y0 = area.y1 + h * (1 - c.stats[j-1].cpu);
			float y1 = area.y1 + h * (1 - c.stats[j].cpu);
			if (x1 >= 2)
				p->draw_line({x0, y0}, {x1, y1});
		}
	}
}

bool is_sleeping(PerfChannelInfo &c) {
	if (c.stats.num == 0)
		return true;
	return c.stats.back().counter == 0;
}


void draw_str_r(Painter *p, float x, float y, const string &s) {
	float w = p->get_str_width(s);
	p->draw_str({x - w, y}, s);
}

void CpuDisplay::draw_table(Painter* p) {
	if (large) {
		float xoff[3] = {200, 270, 340};

		p->set_font_size(10);
		p->set_color(theme.text_soft1);
		draw_str_r(p, xoff[0], 10, "cpu");
		draw_str_r(p, xoff[1], 10, "avg");
		draw_str_r(p, xoff[2], 10, "freq");

		p->set_color(theme.text_soft3);
		p->draw_str({xoff[0], 10}, " %");
		p->draw_str({xoff[1], 10}, " ms");
		p->draw_str({xoff[2], 10}, " Hz");
		
		[[maybe_unused]] int t = 0;
		Array<int> indent;
		color col0;
		[[maybe_unused]] int skip_until_indent = -1;

		float max_avg = 0;
		float max_cpu = 0;
		for (auto &c: channels)
			if (c.stats.num > 0) {
				max_avg = max(max_avg, c.stats.back().avg);
				max_cpu = max(max_cpu, c.stats.back().cpu);
			}


		float y = scroll_offset + 30;
		for (auto &c: channels) {
			if (c.stats.num > 0) {
				if (c.parent < 0)
					col0 = color::interpolate(type_color(c.name), theme.text, 0.5f);
				p->set_color(col0);
				bool highlight = !is_sleeping(c);
				//if (!show_total)
				highlight = (c.stats.back().avg > (max_avg / 10)) or (c.stats.back().cpu > (max_cpu / 10));
				if (!highlight)
					p->set_color(color::interpolate(col0, theme.background, 0.7f));
				float dx = 0;
				if (c.parent >= 0) {
					for (int i=0; i<channels.num; i++)
						if (channels[i].id == c.parent)
							if (i < indent.num) {
								dx = indent[i] + 12;
							}
				}
				string name = channel_title(c);
				p->set_font_size(9);
				float dy = 12;
				p->draw_str({20 + dx, y}, name);
				draw_str_r(p, xoff[0], y, format("%.1f", c.stats.back().cpu * 100));
				draw_str_r(p, xoff[1], y, format("%.2f", c.stats.back().avg * 1000));
				draw_str_r(p, xoff[2], y, format("%.1f", (float)c.stats.back().counter / UPDATE_DT));
				indent.add(dx);
				y += dy;

				/*if (expanded.find(c.id) < 0) {
					skip_until_indent = dx;
				}*/
			} else {
				indent.add(0);
			}
		}
	} else {
		float x0 = area.x1;
		float y0 = area.y1;
		float h = area.height();
	
		p->set_font_size(7);
	
		int t = 0;
		for (auto &c: channels) {
			if (!is_sleeping(c) and (c.parent < 0)) {
				color col = color::interpolate(type_color(c.name), theme.text, 0.5f);
				p->set_color(col);
				p->draw_str({x0 + 7 + (t/2) * 30, y0 + h / 2-10 + (t%2)*12}, format("%2.0f%%", c.stats.back().cpu * 100));
				t ++;
			}
		}
	}
}

void CpuDisplay::on_draw(Painter* p) {
	int h = area.height();
	large = (h > 50);

	draw_background(p);

	p->set_line_width(large ? 2.0f : 1.0f);
	draw_graphs(p);

	draw_table(p);
}

bool CpuDisplay::on_left_button_down(const vec2 &m) {
	if (large)
		return true;
	if (!dlg)
		dlg = new CpuDisplayDialog(session);
	dlg->show();
	return true;
}

bool CpuDisplay::on_mouse_wheel(const vec2 &d) {
	if (large) {
		scroll_offset = min(scroll_offset - d.y * 10, 0.0f);
		request_redraw();
	}
	return true;
}

void ch_sort(Array<PerfChannelInfo> &channels) {
	for (int i=0; i<channels.num; i++)
		for (int j=i+1; j<channels.num; j++) {
			if (channels[i].name.compare(channels[j].name) < 0) {
				channels.swap(i, j);
			} else if ((channels[i].name.compare(channels[j].name) == 0)) { // and (channels[i].stats.back().cpu < channels[j].stats.back().cpu)) {
				channels.swap(i, j);
			}
		}
}

Array<PerfChannelInfo> ch_children(const Array<PerfChannelInfo> &channels, int parent, bool allow_sleeping) {
	Array<PerfChannelInfo> r;
	for (auto &c: channels)
		if (c.parent == parent)
			if (!is_sleeping(c) or allow_sleeping)
				r.add(c);
	ch_sort(r);
	return r;
}

Array<PerfChannelInfo> ch_tree_sort(const Array<PerfChannelInfo> &ch, bool allow_sleeping) {
	auto r = ch_children(ch, -1, allow_sleeping);
	for (int i=0; i<r.num; i++) {
		auto x = ch_children(ch, r[i].id, allow_sleeping);
		foreachi (auto &c, x, j)
			r.insert(c, i+j+1);
	}
	return r;
}

void CpuDisplay::update() {
	channels = ch_tree_sort(perf_mon->get_info(), show_sleeping);
	if (!show_total) {
		for (auto &c: channels)
			if (c.stats.num > 0) {
				auto nodes = ch_children(channels, c.id, false);
				for (auto &n: nodes) {
					c.stats.back().cpu -= n.stats.back().cpu;
					c.stats.back().avg -= n.stats.back().avg;
				}
			}
	}
	request_redraw();
}
