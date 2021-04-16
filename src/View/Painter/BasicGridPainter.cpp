/*
 * BasicGridPainter.cpp
 *
 *  Created on: Apr 16, 2021
 *      Author: michi
 */

#include "BasicGridPainter.h"
#include "../../lib/image/Painter.h"
#include <cmath>

color col_inter(const color a, const color &b, float t);

BasicGridPainter::BasicGridPainter() {
	horizontal = true;
	area = rect::EMPTY;
}


void BasicGridPainter::set_context(const rect &_area, const GridColors &c) {
	area = _area;
	colors = c;
}

void BasicGridPainter::plan_linear(float v0, float v1, float min_dist) {
	ticks.clear();
	labels.clear();

	double w = (double)area.width();
	double x0 = (double)area.x1;
	if (!horizontal) {
		w = (double)area.height();
		x0 = (double)area.y1;
	}
	min_grid_dist = min_dist;
	double dv = min_grid_dist / w * (double)(v1 - v0);
	double ldv = log10(dv);
	double factor = 1;

	// for time...
	if (false) {
		if (ldv > 1.5)
			factor = 1.0/0.6/0.60000001;
		else if (ldv > 0)
			factor = 1.0/0.600000001;
		ldv += log10(factor);
	}
	double exp_s = ceil(ldv);
	double exp_s_mod = exp_s - ldv;
	dv = pow(10, exp_s) / factor;
	int nx0 = ceil(v0 / dv);
	int nx1 = ceil(v1 / dv);

	// don't draw too close to the border...
	if (fabs(x0 + w * (nx0*dv - v0) / (v1 - v0) - x0) < 1.0)
		nx0 ++;
	//if (fabs(x0 + w * (nx1*dv - v0) / (v1 - v0) - (x0+w)) < 1.0)
	//	nx1 --;

	weights[0] = 1;
	weights[1] = min((float)exp_s_mod * 1.8f, 1.0f);
	weights[2] = (float)exp_s_mod;

	/*color c1 = col_inter(colors.bg, colors.fg, exp_s_mod * 0.8f);
	color c2 = col_inter(colors.bg, colors.fg, 0.8f);
	color c1s = col_inter(colors.bg_sel, colors.fg, exp_s_mod * 0.8f);
	color c2s = col_inter(colors.bg_sel, colors.fg_sel, 0.8f);*/

	for (int n=nx0; n<nx1; n++) {
		double v = n * dv;
		int x = x0 + w * (v - v0) / (v1 - v0);
		//c->draw_line(xx, area.y1, xx, area.y2);
		int weight = 2;
		if ((n % 10) == 0)
			weight = 0;
		else if ((n % 5) == 0)
			weight = 1;
		Tick t = {(float)x, (float)v, weight};
		ticks.add(t);
	}
}

void BasicGridPainter::draw(Painter *p) {
	color col[8];
	for (int k=0; k<3; k++)
		col[k] = col_inter(colors.bg, colors.fg, weights[k] * 0.8f);
	p->set_line_width(0.7f);

	if (horizontal) {
		for (auto &t: ticks) {
			p->set_color(col[t.weight]);
			p->draw_line(t.x, area.y1, t.x, area.y2);
		}
	} else {
		for (auto &t: ticks) {
			p->set_color(col[t.weight]);
			p->draw_line(area.x1, t.x, area.x2, t.x);
		}
	}
	p->set_line_width(1);
}


void BasicGridPainter::draw_empty_background(Painter *p) {
	p->set_color(colors.bg);
	p->draw_rect(area);
}
