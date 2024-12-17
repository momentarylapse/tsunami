/*
 * ViewModeCurve.cpp
 *
 *  Created on: 14.11.2015
 *      Author: michi
 */

#include "ViewModeCurve.h"
#include "../audioview/AudioView.h"
#include "../audioview/graph/AudioViewLayer.h"
#include "../audioview/graph/AudioViewTrack.h"
#include "../sidebar/SideBar.h"
#include "../TsunamiWindow.h"
#include "../ColorScheme.h"
#include "../../data/Curve.h"
#include "../../data/Track.h"
#include "../../module/stream/AudioOutput.h"
#include "../../module/audio/SongRenderer.h"
#include "../../lib/image/Painter.h"

namespace tsunami {

ViewModeCurve::ViewModeCurve(AudioView* view) :
	ViewModeDefault(view)
{
	mode_name = "curve";
	_curve = nullptr;
}


void ViewModeCurve::on_start() {
	set_side_bar(SideBar::Index::CurveConsole);
}

AudioViewTrack *ViewModeCurve::cur_vtrack() {
	return view->cur_vtrack();
}

Track *ViewModeCurve::cur_track() {
	return view->cur_track();
}

void ViewModeCurve::left_click_handle_void(AudioViewLayer *vlayer, const vec2 &m) {
	if (target == "")
		return;

	if (!_curve) {
		CurveTarget t;
		t.from_id(target, cur_track());
		_curve = cur_track()->add_curve("", t);
	}

	if (hover().type == HoverData::Type::CurvePointNone) {
		int pos = view->get_mouse_pos(m);
		float value = screen2value(m.y);
		cur_track()->curve_add_point(_curve, pos, value);
	} else if (hover().type == HoverData::Type::CurvePoint) {
		view->mdp_prepare([this] (const vec2& m) {
			int pos = view->get_mouse_pos(m);
			float value = clamp(screen2value(m.y), _curve->min, _curve->max);
			cur_track()->curve_edit_point(_curve, view->cur_selection.index, pos, value);
		}, m);
	}
}

void ViewModeCurve::on_key_down(int k) {
	ViewModeDefault::on_key_down(k);

	if (_curve and (view->cur_selection.type == HoverData::Type::CurvePoint))
		if (k == hui::KEY_DELETE){
			cur_track()->curve_delete_point(_curve, view->cur_selection.index);
			view->cur_selection.clear();
		}
}

void ViewModeCurve::draw_track_data(Painter* c, AudioViewTrack* t) {
	ViewModeDefault::draw_track_data(c, t);

	if (t != cur_vtrack())
		return;

	rect r = t->area;
	if (_curve) {

		// lines
		c->set_line_width(2.0f);
		c->set_color(theme.selection_boundary);
		Array<vec2> pp;
		for (int x=r.x1; x<r.x2; x+=3)
			pp.add(vec2(x, value2screen(_curve->get(cam->screen2sample(x)))));
		c->draw_lines(pp);


		// fill
		c->set_line_width(2.0f);
		color cc = theme.selection_internal;
		cc.a = 0.2f;
		c->set_color(cc);
		pp.add(vec2(r.x2, r.y2));
		pp.add(vec2(r.x1, r.y2));
		c->draw_polygon(pp);

		// points
		foreachi(auto &p, _curve->points, i) {
			float r = 3;
			if ((hover().type == HoverData::Type::CurvePoint) and (i == hover().index)) {
				c->set_color(theme.selection_boundary_hover);
				r = 5;
			} else if ((view->cur_selection.type == HoverData::Type::CurvePoint) and (i == view->cur_selection.index)) {
				// TODO.... selected...
				c->set_color(theme.selection_boundary);
			} else {
				c->set_color(theme.text);
			}
			c->draw_circle({(float)cam->sample2screen(p.pos), value2screen(p.value)}, r);
		}
	}
}

void ViewModeCurve::draw_post(Painter* c) {
	auto *t = cur_vtrack();
	if (t)
		ViewModeDefault::draw_selected_layer_highlight(c, t->area);
}

HoverData ViewModeCurve::get_hover_data(AudioViewLayer *vlayer, const vec2 &m) {
	auto s = vlayer->Node::get_hover_data(m);//ViewModeDefault::get_hover_data(vlayer, mx, my);
	s.type = HoverData::Type::CurvePointNone;

	// curve points
	if (_curve) {
		foreachi(auto &p, _curve->points, i) {
			float x = cam->sample2screen(p.pos);
			float y = value2screen(p.value);
			if ((fabs(m.x - x) < 10) and (fabs(m.y - y) < 10)) {
				s.type = HoverData::Type::CurvePoint;
				s.index = i;
				return s;
			}
		}
	}

	return s;
}

void ViewModeCurve::set_curve_target(const string &id) {
	target = id;
	_curve = nullptr;
	if (cur_track())
		for (auto *c: weak(cur_track()->curves))
			if (c->target.id == target)
				_curve = c;
	view->force_redraw();
}

float ViewModeCurve::value2screen(float value) {
	if (!cur_track())
		return 0;
	if (!_curve)
		return cur_vtrack()->area.y2 - cur_vtrack()->area.height() * value;
	return cur_vtrack()->area.y2 - cur_vtrack()->area.height() * (value - _curve->min) / (_curve->max - _curve->min);
}

float ViewModeCurve::screen2value(float y) {
	if (!cur_vtrack())
		return 0;
	if (!_curve)
		return (cur_vtrack()->area.y2 - y) / cur_vtrack()->area.height();
	return _curve->min + (cur_vtrack()->area.y2 - y) / cur_vtrack()->area.height() * (_curve->max - _curve->min);
}

string ViewModeCurve::get_tip() {
	return "click to add points onto the track,  delete...";
}

}
