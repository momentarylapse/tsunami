/*
 * SignalEditorBackground.cpp
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#include "SignalEditorBackground.h"
#include "SignalEditorTab.h"
#include "../ColorScheme.h"
#include "../audioview/AudioView.h"
#include "../../lib/image/Painter.h"

namespace tsunami {

const float MODULE_GRID = 23;

SignalEditorBackground::SignalEditorBackground(SignalEditorTab *t) {
	align.horizontal = AlignData::Mode::Fill;
	align.vertical = AlignData::Mode::Fill;
	tab = t;
	set_perf_name("background");
}


void SignalEditorBackground::on_draw(Painter *p) {
	p->set_color(theme.background);
	p->draw_rect(area);
	p->set_line_width(0.7f);

	const rect ua = pad->unproject(area);

	float D = MODULE_GRID;
	int i0 = floor(ua.x1 / D);
	int i1 = ceil(ua.x2 / D);
	for (int i=i0; i<=i1; i++) {
		float x = pad->project(vec2(i * D, 0)).x;
		if ((i % 5) == 0)
			p->set_color(color::interpolate(theme.background, theme.grid, 0.5f));
		else
			p->set_color(color::interpolate(theme.background, theme.grid, 0.2f));
		p->draw_line({x, area.y1}, {x, area.y2});
	}
	int j0 = floor(ua.y1 / D);
	int j1 = ceil(ua.y2 / D);
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

}

