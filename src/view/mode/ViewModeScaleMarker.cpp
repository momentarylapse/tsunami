/*
 * ViewModeScaleMarker.cpp
 *
 *  Created on: 26.05.2019
 *      Author: michi
 */

#include "ViewModeScaleMarker.h"
#include "../audioview/AudioView.h"
#include "../../Session.h"
#include "../../EditModes.h"
#include "../../data/TrackMarker.h"
#include "../../data/TrackLayer.h"
#include "../../data/Range.h"
#include "../../lib/hui/language.h"
#include "../../lib/image/Painter.h"
#include "../sidebar/SideBar.h"

namespace tsunami {

ViewModeScaleMarker::ViewModeScaleMarker(AudioView *view) :
	ViewModeDefault(view)
{
	mode_name = "scale-marker";

	scaling = false;
	marker = nullptr;
	layer = nullptr;
}

void ViewModeScaleMarker::on_start() {
	set_side_bar((SideBarIndex)-1);
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
	c->draw_rect(rect(x1, x1 + 30, view->area.y1, view->area.y2));
	c->draw_rect(rect(x2 - 30, x2, view->area.y1, view->area.y2));
}

string ViewModeScaleMarker::get_tip() {
	return _("move selection handles to scale   cancel (Esc)   done (Return)");
}

void ViewModeScaleMarker::on_left_button_up(const vec2 &m) {
	ViewModeDefault::on_left_button_up(m);

	perform_scale();
}

void ViewModeScaleMarker::on_right_button_down(const vec2 &m) {
	session->set_mode(EditMode::Default);
}

void ViewModeScaleMarker::on_mouse_move(const vec2 &m) {
	ViewModeDefault::on_mouse_move(m);
}

void ViewModeScaleMarker::on_key_down(int k) {
	if (k == hui::KEY_ESCAPE)
		session->set_mode(EditMode::Default);
	if (k == hui::KEY_RETURN)
		perform_scale();
}

void ViewModeScaleMarker::perform_scale() {
	if (marker)
		layer->edit_marker(marker, view->sel.range(), marker->text);

	session->set_mode(EditMode::Default);
}

}

