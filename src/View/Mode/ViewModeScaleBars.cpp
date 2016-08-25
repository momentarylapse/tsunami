/*
 * ViewModeBars.cpp
 *
 *  Created on: 13.11.2015
 *      Author: michi
 */

#include "../AudioView.h"
#include "../../lib/hui/hui.h"
#include "../../lib/math/math.h"
#include "ViewModeScaleBars.h"

ViewModeScaleBars::ViewModeScaleBars(AudioView *view) :
	ViewModeDefault(view)
{

	scaling_change = false;
}

ViewModeScaleBars::~ViewModeScaleBars()
{
}

void ViewModeScaleBars::drawPost(Painter *c)
{
	int sx1 = cam->sample2screen(view->sel.range.start());
	int sx2 = cam->sample2screen(view->sel.range.end());
	int sxx1 = clampi(sx1, view->area.x1, view->area.x2);
	int sxx2 = clampi(sx2, view->area.x1, view->area.x2);

	c->setColor(color(0.1f, 1, 1, 1));
	c->drawRect(sxx1, view->area.y1, 30, view->area.height());
	c->drawRect(sxx2 - 30, view->area.y1, 30, view->area.height());
	c->setColor(view->colors.text);
	c->drawStr((sxx1 + sxx2) / 2, (view->area.y1 + view->area.y2) / 2, _("move selection to scale"));
}


void ViewModeScaleBars::startScaling(const Array<int> &sel)
{
	scaling_change = false;
	scaling_sel = sel;
	scaling_range_orig = view->sel.bar_range;
	view->sel_raw = view->sel.bar_range;
	view->updateSelection();
}

void ViewModeScaleBars::onLeftButtonUp()
{
	ViewModeDefault::onLeftButtonUp();

	performScale();
}

void ViewModeScaleBars::onRightButtonDown()
{
	view->setMode(view->mode_default);
}

void ViewModeScaleBars::onMouseMove()
{
	ViewModeDefault::onMouseMove();

	scaling_change = true;
}

void ViewModeScaleBars::onKeyDown(int k)
{
	if (k == KEY_ESCAPE)
		view->setMode(view->mode_default);
}

void ViewModeScaleBars::performScale()
{
	float factor = (float)view->sel.range.length / (float)scaling_range_orig.length;

	song->action_manager->beginActionGroup();
	foreachb(int i, scaling_sel){
		BarPattern b = song->bars[i];
		b.length = (int)((float)b.length * factor);
		song->editBar(i, b, view->bars_edit_data);
	}
	song->action_manager->endActionGroup();

	view->setMode(view->mode_default);
}

