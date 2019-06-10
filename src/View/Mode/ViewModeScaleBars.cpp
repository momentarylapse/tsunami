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
#include "../../Data/Song.h"
#include "../../Data/Rhythm/Bar.h"
#include "../../Session.h"

ViewModeScaleBars::ViewModeScaleBars(AudioView *view) :
	ViewModeDefault(view)
{}

void ViewModeScaleBars::on_start() {
	start_scaling(view->sel.bar_indices(view->song));
}

void ViewModeScaleBars::draw_post(Painter *c) {
	float x1, x2;
	cam->range2screen_clip(view->sel.range, view->area, x1, x2);

	c->set_color(color(0.1f, 1, 1, 1));
	c->draw_rect(x1, view->area.y1, 30, view->area.height());
	c->draw_rect(x2 - 30, view->area.y1, 30, view->area.height());

	string message = _("move right selection handle to scale   cancel (Esc)");
	view->draw_boxed_str(c, (view->song_area.x1 + view->song_area.x2)/2, view->area.y2 - 30, message, view->colors.text_soft1, view->colors.background_track_selection, 0);
}


void ViewModeScaleBars::start_scaling(const Array<int> &sel) {
	scaling_sel = {};
	for (int i=sel[0]; i<=sel.back(); i++)
		scaling_sel.add(i);
	scaling_range_orig = RangeTo(song->bars[sel[0]]->range().start(), song->bars[sel.back()]->range().end());
	view->sel.range = scaling_range_orig;
	view->update_selection();
}

void ViewModeScaleBars::on_left_button_up() {
	ViewModeDefault::on_left_button_up();

	perform_scale();
}

void ViewModeScaleBars::on_right_button_down() {
	session->set_mode("default");
}

void ViewModeScaleBars::on_key_down(int k) {
	if (k == hui::KEY_ESCAPE)
		session->set_mode("default");
	if (k == hui::KEY_RETURN)
		perform_scale();
}

void ViewModeScaleBars::perform_scale() {
	float factor = (float)view->sel.range.length / (float)scaling_range_orig.length;

	song->begin_action_group();
	foreachb(int i, scaling_sel) {
		BarPattern bb = *song->bars[i];
		bb.length = (int)((float)bb.length * factor);
		song->edit_bar(i, bb, Bar::EditMode::STRETCH);
	}
	song->end_action_group();

	session->set_mode("default");
}

