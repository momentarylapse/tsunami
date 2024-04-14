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
#include "../../audioview/AudioView.h"
#include "../../../module/SignalChain.h"
#include "../../../module/Module.h"
#include "../../../lib/math/interpolation.h"

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

	color base_color = tab->signal_color(type, false);

	// curve
	Array<vec2> cc;
	for (float t=0; t<=1.0f; t+=0.025f)
		cc.add(inter.get(t));
	//p->set_color(color::interpolate(base_color, view->colors.background, 0.1f));
	p->set_line_width(4.0f);
	p->set_color(base_color.with_alpha(0.3f));
	//p->set_color(color(1,0.7f,0.2f,0.3f));//base_color);
//	p->draw_lines(cc);
	p->set_color(theme.text_soft1); //base_color);//color::interpolate(base_color, Gray, 0.6f));
	p->set_color(color(1,0.8f,0.3f,1.0f));//base_color);
	p->set_color(White.with_alpha(0.8f));
	p->set_color(theme.text_soft1);
	p->set_line_width(4.0f);
	p->set_line_dash({5, 2}, 0);
	p->draw_lines(cc);
	p->set_line_dash({}, 0);
	p->set_line_width(1);

//	p->set_color(color::interpolate(base_color, theme.text, 0.1f));
	p->set_color(base_color);
	tab->draw_arrow(p, inter.get(0.5f), inter.getTang(0.5f), min(length / 7, 14.0f));
}

bool SignalEditorCable::hover(const vec2 &m) const {
	return false;
}
