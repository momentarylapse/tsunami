/*
 * SignalEditorBackground.cpp
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#include "SignalEditorBackground.h"
#include "SignalEditorTab.h"
#include "../audioview/AudioView.h"


const float MODULE_GRID = 23;

SignalEditorBackground::SignalEditorBackground(SignalEditorTab *t) {
	align.horizontal = AlignData::Mode::FILL;
	align.vertical = AlignData::Mode::FILL;
	tab = t;
	set_perf_name("background");
}


void SignalEditorBackground::on_draw(Painter *p) {
	p->set_color(theme.background);
	p->draw_rect(area);
	p->set_line_width(0.7f);

	auto p0 = pad->unproject({area.x1, area.y1});
	auto p1 = pad->unproject({area.x2, area.y2});

	float D = MODULE_GRID;
	int i0 = floor(p0.x / D);
	int i1 = ceil(p1.x / D);
	for (int i=i0; i<=i1; i++) {
		float x = pad->project(vec2(i * D, 0)).x;
		if ((i % 5) == 0)
			p->set_color(color::interpolate(theme.background, theme.grid, 0.5f));
		else
			p->set_color(color::interpolate(theme.background, theme.grid, 0.2f));
		p->draw_line({x, area.y1}, {x, area.y2});
	}
	int j0 = floor(p0.y / D);
	int j1 = ceil(p1.y / D);
	for (int j=j0; j<=j1; j++) {
		float y = pad->project(vec2(0, j * D)).y;
		if ((j % 5) == 0)
			p->set_color(color::interpolate(theme.background, theme.grid, 0.5f));
		else
			p->set_color(color::interpolate(theme.background, theme.grid, 0.2f));
		p->draw_line({area.x1, y}, {area.x2, y});
	}

	p->set_line_width(1);
}

bool SignalEditorBackground::on_left_button_down(const vec2 &m) {
	tab->select_module(nullptr);
	return true;
}

bool SignalEditorBackground::on_right_button_down(const vec2 &m) {
	tab->popup_chain();
	return true;
}

