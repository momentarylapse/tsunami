/*
 * ViewModeBars.cpp
 *
 *  Created on: 13.11.2015
 *      Author: michi
 */

#include "ViewModeBars.h"
#include "../AudioView.h"
#include "../../lib/hui/hui.h"
#include "../../lib/math/math.h"

ViewModeBars::ViewModeBars(AudioView *view) :
	ViewModeDefault(view)
{

	scaling = false;
	scaling_change = false;
	modify_midi = true;
}

ViewModeBars::~ViewModeBars()
{
}

void ViewModeBars::drawPost(Painter *c)
{
	if (scaling){
		int sx1 = cam->sample2screen(view->sel.range.start());
		int sx2 = cam->sample2screen(view->sel.range.end());
		int sxx1 = clampi(sx1, view->area.x1, view->area.x2);
		int sxx2 = clampi(sx2, view->area.x1, view->area.x2);

		c->setColor(color(0.1f, 1, 1, 1));
		c->drawRect(sxx1, view->area.y1, 30, view->area.height());
		c->drawRect(sxx2 - 30, view->area.y1, 30, view->area.height());
		c->setColor(view->colors.text);
		c->drawStr((sxx1 + sxx2) / 2, (view->area.y1 + view->area.y2) / 2, _("Selektierung ziehen zum Skalieren"));
	}
}


void ViewModeBars::startScaling(const Array<int> &sel)
{
	scaling = true;
	scaling_change = false;
	scaling_sel = sel;
	scaling_range_orig = view->sel.range;
	view->forceRedraw();
}

void ViewModeBars::onLeftButtonUp()
{
	ViewModeDefault::onLeftButtonUp();

	if (scaling)
		view->mode_bars->performScale();
}

void ViewModeBars::onMouseMove()
{
	ViewModeDefault::onMouseMove();

	if (scaling)
		scaling_change = true;
}

void ViewModeBars::performScale()
{
	scaling = false;
	float factor = (float)view->sel.range.num / (float)scaling_range_orig.num;

	song->action_manager->beginActionGroup();
	foreachb(int i, scaling_sel){
		BarPattern b = song->bars[i];
		b.length *= factor;
		song->editBar(i, b, modify_midi);
	}
	song->action_manager->endActionGroup();
}

