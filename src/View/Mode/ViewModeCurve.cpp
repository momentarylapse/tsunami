/*
 * ViewModeCurve.cpp
 *
 *  Created on: 14.11.2015
 *      Author: michi
 */

#include "ViewModeCurve.h"
#include "../AudioView.h"
#include "../Node/AudioViewTrack.h"
#include "../Node/AudioViewLayer.h"
#include "../../Data/Curve.h"
#include "../../Data/Song.h"
#include "../../Device/Stream/AudioOutput.h"
#include "../../TsunamiWindow.h"
#include "../../Module/Audio/SongRenderer.h"
#include "../SideBar/SideBar.h"

ViewModeCurve::ViewModeCurve(AudioView* view) :
	ViewModeDefault(view)
{
	curve = nullptr;
}


void ViewModeCurve::on_start() {
	set_side_bar(SideBar::CURVE_CONSOLE);
}

AudioViewTrack *ViewModeCurve::cur_track() {
	return view->cur_vtrack();
}

void ViewModeCurve::left_click_handle_void(AudioViewLayer *vlayer) {
	if (!curve)
		return;

	if (hover().type == HoverData::Type::CURVE_POINT_NONE) {
		int pos = view->get_mouse_pos();
		float value = screen2value(view->my);
		song->curve_add_point(curve, pos, value);
	} else if (hover().type == HoverData::Type::CURVE_POINT) {
		view->mdp_prepare([=] {
			int pos = view->get_mouse_pos();
			float value = screen2value(view->my);
			song->curve_edit_point(curve, view->cur_selection.index, pos, value);
		});
	}
}

void ViewModeCurve::on_key_down(int k) {
	ViewModeDefault::on_key_down(k);

	if (curve and (view->cur_selection.type == HoverData::Type::CURVE_POINT))
		if (k == hui::KEY_DELETE){
			song->curve_delete_point(curve, view->cur_selection.index);
			view->cur_selection.clear();
		}
}

void ViewModeCurve::draw_track_data(Painter* c, AudioViewTrack* t) {
	ViewModeDefault::draw_track_data(c, t);

	if (t != cur_track())
		return;

	rect r = t->area;
	if (curve) {

		// lines
		c->set_line_width(1.0f);
		c->set_color(view->colors.text);
		Array<complex> pp;
		for (int x=r.x1; x<r.x2; x+=3)
			pp.add(complex(x, value2screen(curve->get(cam->screen2sample(x)))));
		c->draw_lines(pp);

		// points
		foreachi(auto &p, curve->points, i) {
			float r = 3;
			if ((hover().type == HoverData::Type::CURVE_POINT) and (i == hover().index)) {
				c->set_color(view->colors.selection_boundary_hover);
				r = 5;
			} else if ((view->cur_selection.type == HoverData::Type::CURVE_POINT) and (i == view->cur_selection.index)) {
				// TODO.... selected...
				c->set_color(view->colors.selection_boundary);
			} else {
				c->set_color(view->colors.text);
			}
			c->draw_circle(cam->sample2screen(p.pos), value2screen(p.value), r);
		}
	}
}

void ViewModeCurve::draw_post(Painter* c) {
	auto *t = cur_track();
	if (!t)
		return;
	color col = view->colors.text;
	col.a = 0.1f;
	float d = 12;
	c->set_color(col);
	c->draw_rect(view->song_area().x1, t->area.y1-d, view->song_area().width(), d);
	c->draw_rect(view->song_area().x1, t->area.y2, view->song_area().width(), d);
	d = 2;
	col.a = 0.7f;
	c->set_color(col);
	c->draw_rect(view->song_area().x1, t->area.y1-d, view->song_area().width(), d);
	c->draw_rect(view->song_area().x1, t->area.y2, view->song_area().width(), d);
}

HoverData ViewModeCurve::get_hover_data(AudioViewLayer *vlayer, float mx, float my) {
	auto s = vlayer->ViewNode::get_hover_data(mx, my);//ViewModeDefault::get_hover_data(vlayer, mx, my);
	s.type = HoverData::Type::CURVE_POINT_NONE;

	// curve points
	if (curve) {
		foreachi(auto &p, curve->points, i) {
			float x = cam->sample2screen(p.pos);
			float y = value2screen(p.value);
			if ((fabs(mx - x) < 10) and (fabs(my - y) < 10)) {
				s.type = HoverData::Type::CURVE_POINT;
				s.index = i;
				return s;
			}
		}
	}

	return s;
}

void ViewModeCurve::set_curve(Curve* c) {
	curve = c;
	view->force_redraw();
}

float ViewModeCurve::value2screen(float value) {
	if ((!curve) or (!cur_track()))
		return 0;
	return cur_track()->area.y2 - cur_track()->area.height() * (value - curve->min) / (curve->max - curve->min);
}

float ViewModeCurve::screen2value(float y) {
	if ((!curve) or (!cur_track()))
		return 0;
	return curve->min + (cur_track()->area.y2 - y) / cur_track()->area.height() * (curve->max - curve->min);
}

string ViewModeCurve::get_tip() {
	return "click to add points onto the track,  delete...";
}
