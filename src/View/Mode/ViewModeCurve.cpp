/*
 * ViewModeCurve.cpp
 *
 *  Created on: 14.11.2015
 *      Author: michi
 */

#include "ViewModeCurve.h"
#include "../AudioView.h"
#include "../AudioViewTrack.h"
#include "../../Data/Curve.h"
#include "../../Data/Song.h"
#include "../../Device/OutputStream.h"
#include "../../TsunamiWindow.h"
#include "../../Module/Audio/SongRenderer.h"

ViewModeCurve::ViewModeCurve(AudioView* view) :
	ViewModeDefault(view)
{
	curve = nullptr;
	cur_track = nullptr;
}

ViewModeCurve::~ViewModeCurve()
{
}

void ViewModeCurve::on_left_button_down()
{
	ViewModeDefault::on_left_button_down();


	if (curve and (hover->type == Selection::Type::CURVE_POINT_NONE)){
		int pos = cam->screen2sample(view->mx);
		float value = screen2value(view->my);
		song->curve_add_point(curve, pos, value);
		//view->forceRedraw();
	}
}

void ViewModeCurve::on_left_button_up()
{
	ViewModeDefault::on_left_button_up();
}

void ViewModeCurve::on_mouse_move()
{
	ViewModeDefault::on_mouse_move();

	if (hui::GetEvent()->lbut){
		if (curve and (hover->type == Selection::Type::CURVE_POINT)){
			int pos = cam->screen2sample(view->mx);
			float value = clampf(screen2value(view->my), curve->min, curve->max);
			song->curve_edit_point(curve, hover->index, pos, value);
			//view->forceRedraw();
		}
	}
}

void ViewModeCurve::on_key_down(int k)
{
	ViewModeDefault::on_key_down(k);

	if (curve and (hover->type == Selection::Type::CURVE_POINT))
		if (k == hui::KEY_DELETE){
			song->curve_delete_point(curve, hover->index);
			hover->clear();
			//view->forceRedraw();
		}
}

void ViewModeCurve::draw_track_data(Painter* c, AudioViewTrack* t)
{
	ViewModeDefault::draw_track_data(c, t);

	if (t->track != view->cur_track())
		return;

	cur_track = t;

	rect r = t->area;
	if (curve){

		// lines
		c->set_line_width(1.0f);
		c->set_color(view->colors.text);
		Array<complex> pp;
		for (int x=r.x1; x<r.x2; x+=3)
			pp.add(complex(x, value2screen(curve->get(cam->screen2sample(x)))));
		c->draw_lines(pp);

		// points
		foreachi(Curve::Point &p, curve->points, i){
			if ((hover->type == Selection::Type::CURVE_POINT) and (i == hover->index))
				c->set_color(view->colors.selection_boundary_hover);
			else if ((hover->type == Selection::Type::CURVE_POINT) and (i == hover->index))
				// TODO.... selected...
				c->set_color(view->colors.selection_boundary);
			else
				c->set_color(view->colors.text);
			c->draw_circle(cam->sample2screen(p.pos), value2screen(p.value), 3);
		}
	}
}

Selection ViewModeCurve::get_hover()
{
	Selection s = ViewModeDefault::get_hover();
	int mx = view->mx;
	int my = view->my;

	// curve points
	if ((s.type == s.Type::LAYER) and curve){
		foreachi(Curve::Point &p, curve->points, i){
			float x = cam->sample2screen(p.pos);
			float y = value2screen(p.value);
			if ((fabs(mx - x) < 10) and (fabs(my - y) < 10)){
				s.type = Selection::Type::CURVE_POINT;
				s.index = i;
				return s;
			}
		}
	}
	if (s.type == s.Type::LAYER)
		s.type = Selection::Type::CURVE_POINT_NONE;

	return s;
}

void ViewModeCurve::setCurve(Curve* c)
{
	curve = c;
	view->force_redraw();
}

float ViewModeCurve::value2screen(float value)
{
	if ((!curve) or (!cur_track))
		return 0;
	return cur_track->area.y2 - cur_track->area.height() * (value - curve->min) / (curve->max - curve->min);
}

float ViewModeCurve::screen2value(float y)
{
	if ((!curve) or (!cur_track))
		return 0;
	return curve->min + (cur_track->area.y2 - y) / cur_track->area.height() * (curve->max - curve->min);
}
