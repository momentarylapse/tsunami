/*
 * ViewModeScaleMarker.cpp
 *
 *  Created on: 26.05.2019
 *      Author: michi
 */

#include "ViewModeScaleMarker.h"
#include "../AudioView.h"
#include "../../Session.h"
#include "../../Data/TrackMarker.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/Range.h"


ViewModeScaleMarker::ViewModeScaleMarker(AudioView *view) :
	ViewModeDefault(view)
{

	scaling = false;
	marker = nullptr;
	layer = nullptr;
}

void ViewModeScaleMarker::on_start() {
	scaling = false;
	layer = view->hover_before_leave.layer();
	marker = view->hover_before_leave.marker;

	if (marker) {
		view->sel.range_raw = marker->range;
		view->update_selection();
	} else {
		session->e(_("no marker selected"));
	}
}

void ViewModeScaleMarker::draw_post(Painter *c) {
	float x1, x2;
	cam->range2screen_clip(view->sel.range(), view->area, x1, x2);

	c->set_color(color(0.1f, 1, 1, 1));
	c->draw_rect(x1, view->area.y1, 30, view->area.height());
	c->draw_rect(x2 - 30, view->area.y1, 30, view->area.height());
}

string ViewModeScaleMarker::get_tip() {
	return _("move selection handles to scale   cancel (Esc)   done (Return)");
}

void ViewModeScaleMarker::on_left_button_up() {
	ViewModeDefault::on_left_button_up();

	perform_scale();
}

void ViewModeScaleMarker::on_right_button_down() {
	session->set_mode("default");
}

void ViewModeScaleMarker::on_mouse_move() {
	ViewModeDefault::on_mouse_move();
}

void ViewModeScaleMarker::on_key_down(int k) {
	if (k == hui::KEY_ESCAPE)
		session->set_mode("default");
	if (k == hui::KEY_RETURN)
		perform_scale();
}

void ViewModeScaleMarker::perform_scale() {
	if (marker)
		layer->edit_marker(marker, view->sel.range(), marker->text);

	session->set_mode("default");
}

