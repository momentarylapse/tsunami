/*
 * SignalEditorBackground.cpp
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#include "SignalEditorBackground.h"
#include "SignalEditorTab.h"
#include "../../AudioView.h"


const float MODULE_GRID = 23;

SignalEditorBackground::SignalEditorBackground(SignalEditorTab *t) {
	align.horizontal = AlignData::Mode::FILL;
	align.vertical = AlignData::Mode::FILL;
	tab = t;
}


void SignalEditorBackground::on_draw(Painter *p) {
	int w = area.width();
	int h = area.height();
	auto view = tab->view;
	p->set_color(view->colors.background);
	p->draw_rect(area);
	p->set_line_width(0.7f);


	float rot[4] = {1,0,0,1};
	p->set_transform(rot, tab->view_offset);

	float D = MODULE_GRID;
	int i0 = -tab->view_offset.x / D;
	int i1 = (w - tab->view_offset.x) / D;
	for (int i=i0; i<=i1; i++) {
		float x = i * D;
		if ((i % 5) == 0)
			p->set_color(color::interpolate(view->colors.background, view->colors.grid, 0.5f));
		else
			p->set_color(color::interpolate(view->colors.background, view->colors.grid, 0.2f));
		p->draw_line(x, -tab->view_offset.y, x, h - tab->view_offset.y);
	}
	int j0 = -tab->view_offset.y / D;
	int j1 = (h - tab->view_offset.y) / D;
	for (int j=j0; j<=j1; j++) {
		float y = j * D;
		if ((j % 5) == 0)
			p->set_color(color::interpolate(view->colors.background, view->colors.grid, 0.5f));
		else
			p->set_color(color::interpolate(view->colors.background, view->colors.grid, 0.2f));
		p->draw_line(-tab->view_offset.x, y, w - tab->view_offset.x, y);
	}

	p->set_line_width(1);
	float rot0[] = {1,0,0,1};
	p->set_transform(rot0, complex::ZERO);
}

