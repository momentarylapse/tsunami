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
#include "../../../Module/SignalChain.h"
#include "../../../Module/Module.h"
#include "../../AudioView.h"

SignalEditorCable::SignalEditorCable(SignalEditorTab *t, const Cable &c) : scenegraph::NodeRel(0,0,0,0) {
	tab = t;
	source = c.source;
	target = c.target;
	source_port = c.source_port;
	target_port = c.target_port;
	type = c.type;
}

void SignalEditorCable::on_draw(Painter *p) {
	auto ms = tab->get_module(source);
	auto mt = tab->get_module(target);
	auto ps = ms->out[source_port];
	auto pt = mt->in[target_port];

	complex p0 = complex(ps->area.mx(), ps->area.my());
	complex p1 = complex(pt->area.mx(), pt->area.my());

	float length = (p1 - p0).abs();
	Interpolator<complex> inter(Interpolator<complex>::TYPE_CUBIC_SPLINE);
	inter.add2(p0, complex(length,0));
	inter.add2(p1, complex(length,0));

	color base_color = tab->signal_color(type, false);

	// curve
	p->set_color(base_color);
	//p->set_color(color::interpolate(base_color, view->colors.background, 0.1f));
	p->set_line_width(2.0f);
	p->set_line_dash({5, 2}, 0);
	Array<complex> cc;
	for (float t=0; t<=1.0f; t+=0.025f)
		cc.add(inter.get(t));
	p->draw_lines(cc);
	p->set_line_dash({}, 0);
	p->set_line_width(1);

	p->set_color(color::interpolate(base_color, tab->view->colors.text, 0.1f));
	//p->set_color(base_color);
	tab->draw_arrow(p, inter.get(0.5f), inter.getTang(0.5f), min(length / 7, 14.0f));
}
