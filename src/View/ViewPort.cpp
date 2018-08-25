/*
 * ViewPort.cpp
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#include "ViewPort.h"
#include "AudioView.h"
#include "../lib/math/math.h"

const float ViewPort::BORDER_FACTOR = 1.0f / 25.0f;
const float ViewPort::BORDER_FACTOR_RIGHT = ViewPort::BORDER_FACTOR * 8;

ViewPort::ViewPort(AudioView *v)
{
	view = v;
	scale = 1.0f;
	pos = 0;
	pos_target = 0;
	pos_pre_animation = 0;
	animation_time = -1;
	animation_non_linearity = 0;
}


double ViewPort::screen2sample(double _x)
{
	return (_x - view->song_area.x1) / scale + pos;
}

double ViewPort::sample2screen(double s)
{
	return view->song_area.x1 + (s - pos) * scale;
}

double ViewPort::dsample2screen(double ds)
{
	return ds * scale;
}

void ViewPort::zoom(float f)
{
	// max zoom: 20 pixel per sample
	double scale_new = clampf(scale * f, 0.000001, 20.0);

	pos += (view->mx - view->song_area.x1) * (1.0/scale - 1.0/scale_new);
	pos_target = pos_pre_animation = pos;
	scale = scale_new;
	view->notify(view->MESSAGE_VIEW_CHANGE);
	view->force_redraw();
}

void ViewPort::move(float dpos)
{
	/*pos += dpos;
	view->notify(view->MESSAGE_VIEW_CHANGE);
	view->forceRedraw();*/
	set_target(pos_target + dpos, 0.0f);
}

// nonlin=0   feels "faster", more responsive, good for continuous human controls
// nonlin=0.7 feels smoother, good for automatic single jumps
void ViewPort::set_target(float target, float nonlin)
{
	pos_pre_animation = pos;
	pos_target = target;
	animation_time = 0;
	animation_non_linearity = nonlin;
	view->notify(view->MESSAGE_VIEW_CHANGE);
	view->force_redraw();
}

void ViewPort::update(float dt)
{
	if (animation_time < 0)
		return;
	animation_time += dt;
	double t = animation_time;
	if (t >= 1){
		pos = pos_target;
		animation_time = -1;
	}else{
		float s = animation_non_linearity;
		t = s*(-2*t*t*t + 3*t*t) + (1-s) * t;
		pos = t * pos_target + (1-t) * pos_pre_animation;
	}
	view->notify(view->MESSAGE_VIEW_CHANGE);
	view->force_redraw();
}

bool ViewPort::needs_update()
{
	return (animation_time >= 0);
}

Range ViewPort::range()
{
	return Range(pos, view->song_area.width() / scale);
}

void ViewPort::make_sample_visible(int sample)
{
	double x = sample2screen(sample);
	float dx = view->song_area.width() * BORDER_FACTOR;
	float dxr = view->song_area.width() * BORDER_FACTOR_RIGHT;
	if ((x > view->song_area.x2 - dxr) or (x < view->song_area.x1 + dx)){
		//pos = sample - view->area.width() / scale * BORDER_FACTOR;
		set_target(sample - view->song_area.width() / scale * BORDER_FACTOR, 0.7f);
		//view->notify(view->MESSAGE_VIEW_CHANGE);
		//view->forceRedraw();
	}
}

void ViewPort::show(Range &r)
{
	// mapping target area
	float x0 = view->song_area.x1;
	float x1 = view->song_area.x2;
	if (x1 - x0 > 800)
		x0 += view->TRACK_HANDLE_WIDTH;
	float w = x1 - x0;
	x0 += w * BORDER_FACTOR;
	x1 -= w * BORDER_FACTOR;


	// map r into (x0,x1)
	scale = (x1 - x0) / (double)r.length;
	pos = (double)r.start() - (x0 - view->song_area.x1) / scale;
	pos_pre_animation = pos;
	pos_target = pos;
	view->notify(view->MESSAGE_VIEW_CHANGE);
	view->force_redraw();
}

