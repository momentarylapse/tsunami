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
#include "../../Device/OutputStream.h"
#include "../../TsunamiWindow.h"

ViewModeCurve::ViewModeCurve(AudioView* view) :
	ViewModeDefault(view)
{
	curve = NULL;
	cur_track = NULL;
}

ViewModeCurve::~ViewModeCurve()
{
}

void ViewModeCurve::onLeftButtonDown()
{
	ViewModeDefault::onLeftButtonDown();


	if ((curve) and (hover->type == Selection::TYPE_TRACK)){
		curve->add(cam->screen2sample(view->mx), screen2value(view->my));
		view->forceRedraw();
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
		if ((curve) and (hover->type == Selection::TYPE_CURVE_POINT)){
			curve->points[hover->index].pos = cam->screen2sample(view->mx);
			curve->points[hover->index].value = clampf(screen2value(view->my), curve->min, curve->max);
			view->forceRedraw();
		}
	}
}

void ViewModeCurve::onKeyDown(int k)
{
	ViewModeDefault::onKeyDown(k);

	if ((curve) and (hover->type == Selection::TYPE_CURVE_POINT))
		if (k == hui::KEY_DELETE){
			curve->points.erase(hover->index);
			hover->clear();
			hover->clear();
			view->forceRedraw();
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
			if ((hover->type == Selection::TYPE_CURVE_POINT) and (i == hover->index))
				c->setColor(view->colors.selection_boundary_hover);
			else if ((hover->type == Selection::TYPE_CURVE_POINT) and (i == hover->index))
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
	Selection s;
	int mx = view->mx;
	int my = view->my;
	s.pos = view->cam.screen2sample(mx);

	// track?
	foreachi(AudioViewTrack *t, view->vtrack, i){
		if (view->mouseOverTrack(t)){
			s.vtrack = t;
			s.index = i;
			s.track = t->track;
			s.type = Selection::TYPE_TRACK;
			if (view->mx < t->area.x1 + view->TRACK_HANDLE_WIDTH)
				s.type = Selection::TYPE_TRACK_HANDLE;
		}
	}

	// selection boundaries?
	if (view->mouse_over_time(view->sel.range.end())){
		s.type = Selection::TYPE_SELECTION_END;
		return s;
	}
	if (view->mouse_over_time(view->sel.range.start())){
		s.type = Selection::TYPE_SELECTION_START;
		return s;
	}
	if (view->stream->isPlaying()){
		if (view->mouse_over_time(view->stream->getPos())){
			s.type = Selection::TYPE_PLAYBACK;
			return s;
		}
	}

	// mute button?
	if (s.track){
		AudioViewTrack *t = s.vtrack;
		if ((mx >= t->area.x1 + 5) and (mx < t->area.x1 + 17) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_MUTE;
			return s;
		}
		if ((song->tracks.num > 1) and (mx >= t->area.x1 + 22) and (mx < t->area.x1 + 34) and (my >= t->area.y1 + 22) and (my < t->area.y1 + 34)){
			s.type = Selection::TYPE_SOLO;
			return s;
		}
	}

	// sub?
	if (s.track){

		// markers
		for (int i=0; i<min(s.track->markers.num, view->vtrack[s.index]->marker_areas.num); i++){
			if (view->vtrack[s.index]->marker_areas[i].inside(mx, my)){
				s.type = Selection::TYPE_MARKER;
				s.index = i;
				return s;
			}
		}

		// TODO: prefer selected subs
		for (SampleRef *ss : s.track->samples){
			int offset = view->mouseOverSample(ss);
			if (offset >= 0){
				s.sample = ss;
				s.type = Selection::TYPE_SAMPLE;
				s.sample_offset = offset;
				return s;
			}
		}
	}

	// curve points
	if ((s.track) and (curve)){
		foreachi(Curve::Point &p, curve->points, i){
			float x = cam->sample2screen(p.pos);
			float y = value2screen(p.value);
			if ((fabs(mx - x) < 10) and (fabs(my - y) < 10)){
				s.type = Selection::TYPE_CURVE_POINT;
				s.index = i;
				return s;
			}
		}
	}

	// time scale
	if (my < view->TIME_SCALE_HEIGHT){
		s.type = Selection::TYPE_TIME;
		return s;
	}

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
