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

void ViewModeCurve::onLeftButtonDown()
{
	ViewModeDefault::onLeftButtonDown();


	if (curve and (hover->type == Selection::Type::CURVE_POINT_NONE)){
		int pos = cam->screen2sample(view->mx);
		float value = screen2value(view->my);
		song->curveAddPoint(curve, pos, value);
		//view->forceRedraw();
	}
}

void ViewModeCurve::onLeftButtonUp()
{
	ViewModeDefault::onLeftButtonUp();
}

void ViewModeCurve::onMouseMove()
{
	ViewModeDefault::onMouseMove();

	if (hui::GetEvent()->lbut){
		if (curve and (hover->type == Selection::Type::CURVE_POINT)){
			int pos = cam->screen2sample(view->mx);
			float value = clampf(screen2value(view->my), curve->min, curve->max);
			song->curveEditPoint(curve, hover->index, pos, value);
			//view->forceRedraw();
		}
	}
}

void ViewModeCurve::onKeyDown(int k)
{
	ViewModeDefault::onKeyDown(k);

	if (curve and (hover->type == Selection::Type::CURVE_POINT))
		if (k == hui::KEY_DELETE){
			song->curveDeletePoint(curve, hover->index);
			hover->clear();
			//view->forceRedraw();
		}
}

void ViewModeCurve::drawTrackData(Painter* c, AudioViewTrack* t)
{
	ViewModeDefault::drawTrackData(c, t);

	if (t->track != view->cur_track)
		return;

	cur_track = t;

	rect r = t->area;
	if (curve){

		// lines
		c->setAntialiasing(true);
		c->setLineWidth(1.0f);
		c->setColor(view->colors.text);
		Array<complex> pp;
		for (int x=r.x1; x<r.x2; x+=3)
			pp.add(complex(x, value2screen(curve->get(cam->screen2sample(x)))));
		c->drawLines(pp);

		// points
		foreachi(Curve::Point &p, curve->points, i){
			if ((hover->type == Selection::Type::CURVE_POINT) and (i == hover->index))
				c->setColor(view->colors.selection_boundary_hover);
			else if ((hover->type == Selection::Type::CURVE_POINT) and (i == hover->index))
				// TODO.... selected...
				c->setColor(view->colors.selection_boundary);
			else
				c->setColor(view->colors.text);
			c->drawCircle(cam->sample2screen(p.pos), value2screen(p.value), 3);
		}
	}
}

Selection ViewModeCurve::getHover()
{
	Selection s = ViewModeDefault::getHover();
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
	view->forceRedraw();
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
