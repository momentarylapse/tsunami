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
#include "../ColorScheme.h"
#include "../../module/SignalChain.h"
#include "../../data/base.h"
#include "../../lib/math/interpolation.h"
#include "../../lib/image/Painter.h"
#include "../helper/Drawing.h"

namespace tsunami {

SignalEditorCable::SignalEditorCable(SignalEditorTab *t, const Cable &c) : scenegraph::NodeRel({0,0},0,0) {
	tab = t;
	source = c.source;
	target = c.target;
	source_port = c.source_port;
	target_port = c.target_port;
	type = c.type;
	set_perf_name("se:cable");
}

struct CableCurve {
	Interpolator<vec2> inter;
	float length;
	vec2 p0, p1;
	Array<vec2> cc;
};

static CableCurve cable_curve(const SignalEditorCable* cable) {
	auto ms = cable->tab->get_module(cable->source);
	auto mt = cable->tab->get_module(cable->target);
	auto ps = ms->out[cable->source_port];
	auto pt = mt->in[cable->target_port];

	vec2 p0 = ps->area.center();
	vec2 p1 = pt->area.center();

	float length = (p1 - p0).length();
	Interpolator<vec2> inter(Interpolator<vec2>::TYPE_CUBIC_SPLINE);
	inter.add2(p0, vec2(length,0));
	inter.add2(p1, vec2(length,0));

	Array<vec2> cc;
	for (float t=0; t<=1.0f; t+=0.025f)
		cc.add(inter.get(t));

	return {inter, length, p0, p1, cc};
}


void SignalEditorCable::on_draw(Painter *p) {
	auto curve = cable_curve(this);
	bool hover = is_cur_hover();

	color base_color = SignalEditorTab::signal_color(type, false);

	// curve
	color col = theme.text_soft1;
	if (hover)
		col = theme.hoverify(col);
	p->set_color(col);
	p->set_line_width(hover ? 4.5f : 3.5f);
	if (type == SignalType::Midi)
		p->set_line_dash({6, 3}, 0);
	else if (type == SignalType::Beats)
		p->set_line_dash({3, 6}, 0);
	p->draw_lines(curve.cc);
	p->set_line_dash({}, 0);
	p->set_line_width(1);

	float arrow_len = min(curve.length / 7, 14.0f);
	p->set_color(theme.background);
	p->draw_circle(curve.p0, 10);
	p->draw_circle(curve.p1, 10);
	const vec2 m = curve.inter.get(0.5f);
	const vec2 dir = curve.inter.get_derivative(0.5f).normalized() * arrow_len * 2 * (hover ? 1.2f : 1.0f);
	p->draw_circle(m, arrow_len * 1.3f);
	p->set_color(base_color);
	draw_arrow_head(p, m + dir / 2, dir);
}

bool SignalEditorCable::has_hover(const vec2 &m) const {
	auto curve = cable_curve(this);
	constexpr float R = 20.0f;
	for (const auto& p: curve.cc)
		if ((p - m).length_sqr() < R*R)
			return true;
	return false;
}

string SignalEditorCable::get_tip() const {
	return "Cable: " + signal_type_name(type);
}


}
