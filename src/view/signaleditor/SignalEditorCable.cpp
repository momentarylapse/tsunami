/*
 * SignalEditorCable.cpp
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#include "SignalEditorCable.h"
#include "SignalEditorTab.h"
#include "SignalEditorModule.h"
#include "SignalEditorPort.h"
#include "../audioview/AudioView.h"
#include "../../module/SignalChain.h"
#include "../../data/base.h"
#include "../../lib/math/interpolation.h"

namespace tsunami {

SignalEditorCable::SignalEditorCable(SignalEditorTab *t, const Cable &c) : scenegraph::NodeRel({0,0},0,0) {
	align.dz = -10;
	tab = t;
	source = c.source;
	target = c.target;
	source_port = c.source_port;
	target_port = c.target_port;
	type = c.type;
	set_perf_name("se:cable");
}

void SignalEditorCable::on_draw(Painter *p) {
	auto ms = tab->get_module(source);
	auto mt = tab->get_module(target);
	auto ps = ms->out[source_port];
	auto pt = mt->in[target_port];

	vec2 p0 = ps->area.center();
	vec2 p1 = pt->area.center();

	float length = (p1 - p0).length();
	Interpolator<vec2> inter(Interpolator<vec2>::TYPE_CUBIC_SPLINE);
	inter.add2(p0, vec2(length,0));
	inter.add2(p1, vec2(length,0));

	color base_color = SignalEditorTab::signal_color(type, false);

	// curve
	Array<vec2> cc;
	for (float t=0; t<=1.0f; t+=0.025f)
		cc.add(inter.get(t));
	p->set_color(theme.text_soft1);
	p->set_line_width(3.5f);
	if (type == SignalType::Midi)
		p->set_line_dash({6, 3}, 0);
	else if (type == SignalType::Beats)
		p->set_line_dash({3, 6}, 0);
	p->draw_lines(cc);
	p->set_line_dash({}, 0);
	p->set_line_width(1);

	float arrow_len = min(length / 7, 14.0f);
	p->set_color(theme.background);
	p->draw_circle(p0, 10);
	p->draw_circle(p1, 10);
	p->draw_circle(inter.get(0.5f), arrow_len * 1.3f);
	p->set_color(base_color);
	SignalEditorTab::draw_arrow(p, inter.get(0.5f), inter.getTang(0.5f), arrow_len);
}

bool SignalEditorCable::has_hover(const vec2 &m) const {
	return false;
}

}
