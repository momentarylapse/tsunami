/*
 * ViewPort.cpp
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#include "ViewPort.h"
#include "AudioView.h"
#include "../lib/math/math.h"

const float ViewPort::BORDER_FACTOR = 1.0f / 15.0f;

ViewPort::ViewPort(AudioView *v)
{
	view = v;
	scale = 1.0f;
	pos = 0;
}


double ViewPort::screen2sample(double _x)
{
	return (_x - view->area.x1) / scale + pos;
}

double ViewPort::sample2screen(double s)
{
	return view->area.x1 + (s - pos) * scale;
}

double ViewPort::dsample2screen(double ds)
{
	return ds * scale;
}

void ViewPort::zoom(float f)
{
	// max zoom: 20 pixel per sample
	double scale_new = clampf(scale * f, 0.000001, 20.0);

	pos += (view->mx - view->area.x1) * (1.0/scale - 1.0/scale_new);
	scale = scale_new;
	view->notify(view->MESSAGE_VIEW_CHANGE);
	view->forceRedraw();
}

void ViewPort::move(float dpos)
{
	pos += dpos;
	view->notify(view->MESSAGE_VIEW_CHANGE);
	view->forceRedraw();
}

Range ViewPort::range()
{
	return Range(pos, view->area.width() / scale);
}

void ViewPort::makeSampleVisible(int sample)
{
	double x = sample2screen(sample);
	if ((x > view->area.x2) or (x < view->area.x1)){
		pos = sample - view->area.width() / scale * BORDER_FACTOR;
		view->notify(view->MESSAGE_VIEW_CHANGE);
		view->forceRedraw();
	}
}

void ViewPort::show(Range &r)
{
	int border = r.num * BORDER_FACTOR;
	r.offset -= border;
	r.num += border * 2;
	scale = view->area.width() / (double)r.length();
	pos = (double)r.start();
	view->notify(view->MESSAGE_VIEW_CHANGE);
	view->forceRedraw();
}

