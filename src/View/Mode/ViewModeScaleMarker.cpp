/*
 * ViewModeScaleMarker.cpp
 *
 *  Created on: 26.05.2019
 *      Author: michi
 */

#include "ViewModeScaleMarker.h"
#include "../AudioView.h"
#include "../Selection.h"
#include "../../Session.h"
#include "../../Data/TrackMarker.h"
#include "../../Data/Track.h"
#include "../../Data/Range.h"


ViewModeScaleMarker::ViewModeScaleMarker(AudioView *view) :
	ViewModeDefault(view)
{

	scaling = false;
	marker = nullptr;
	track = nullptr;
}

void ViewModeScaleMarker::on_start()
{
	scaling = false;
	track = view->hover.track;
	marker = view->hover.marker;

	if (marker){
		view->sel.range = marker->range;
		view->update_selection();
	}else{
		session->e(_("no marker selected"));
	}
}

void ViewModeScaleMarker::draw_post(Painter *c)
{
	float x1, x2;
	cam->range2screen_clip(view->sel.range, view->area, x1, x2);

	c->set_color(color(0.1f, 1, 1, 1));
	c->draw_rect(x1, view->area.y1, 30, view->area.height());
	c->draw_rect(x2 - 30, view->area.y1, 30, view->area.height());

	string message = _("move right selection handle to scale   cancel (Esc)");
	view->draw_boxed_str(c, (view->song_area.x1 + view->song_area.x2)/2, view->area.y2 - 30, message, view->colors.text_soft1, view->colors.background_track_selection, 0);
}

void ViewModeScaleMarker::on_left_button_up()
{
	ViewModeDefault::on_left_button_up();

	perform_scale();
}

void ViewModeScaleMarker::on_right_button_down()
{
	session->set_mode("default");
}

void ViewModeScaleMarker::on_mouse_move()
{
	ViewModeDefault::on_mouse_move();
}

void ViewModeScaleMarker::on_key_down(int k)
{
	if (k == hui::KEY_ESCAPE)
		session->set_mode("default");
}

void ViewModeScaleMarker::perform_scale()
{
	if (marker){
		track->edit_marker(marker, view->sel.range, marker->text);
	}

	session->set_mode("default");
}

