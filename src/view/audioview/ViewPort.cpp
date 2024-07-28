/*
 * ViewPort.cpp
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#include "ViewPort.h"
#include "../ColorScheme.h"
#include "../../data/Range.h"
#include "../../lib/math/math.h"

namespace tsunami {

const float ViewPort::BorderFactor = 1.0f / 25.0f;
const float ViewPort::BorderFactorRight = ViewPort::BorderFactor * 8;

ViewPort::ViewPort() {
	pixels_per_sample = 1.0f;
	pos = 0;
	pos_target = 0;
	pos_pre_animation = 0;
	animation_time = -1;
	animation_non_linearity = 0;
	scale_y = 1.0f;
	area = rect::ID;
}

void ViewPort::__init__() {
	new(this) ViewPort();
}


double ViewPort::screen2sample(double _x) {
	return (_x - area.x1) / pixels_per_sample + pos;
}

double ViewPort::sample2screen(double s) {
	return area.x1 + (s - pos) * pixels_per_sample;
}

double ViewPort::dsample2screen(double ds) {
	return ds * pixels_per_sample;
}

double ViewPort::dscreen2sample(double dx) {
	return dx / pixels_per_sample;
}

float ViewPort::screen2sample_f(float _x) {
	return screen2sample(_x);
}

float ViewPort::sample2screen_f(float _x) {
	return sample2screen(_x);
}


void ViewPort::range2screen(const Range &r, float &x1, float &x2) {
	x1 = sample2screen((double)r.start());
	x2 = sample2screen((double)r.end());
}

void ViewPort::range2screen_clip(const Range &r, const rect &area, float &x1, float &x2) {
	range2screen(r, x1, x2);
	x1 = clamp(x1, area.x1, area.x2);
	x2 = clamp(x2, area.x1, area.x2);
}

void ViewPort::zoom(float f, float mx) {
	// max zoom: 20 pixel per sample
	double scale_new = clamp(pixels_per_sample * f, 0.000001, 20.0);

	pos += (mx - area.x1) * (1.0/pixels_per_sample - 1.0/scale_new);
	pos_target = pos_pre_animation = pos;
	pixels_per_sample = scale_new;
	out_changed.notify();
}

void ViewPort::move(float dpos) {
	set_target(pos_target + dpos, 0.0f);
}

// nonlin=0   feels "faster", more responsive, good for continuous human controls
// nonlin=0.7 feels smoother, good for automatic single jumps
void ViewPort::set_target(float target, float nonlin) {
	pos_pre_animation = pos;
	pos_target = target;
	animation_time = 0;
	animation_non_linearity = nonlin;
	out_changed.notify();
}

void ViewPort::update(float dt) {
	if (animation_time < 0)
		return;
	animation_time += dt;
	double t = animation_time;
	if (t >= 1) {
		pos = pos_target;
		animation_time = -1;
	} else {
		float s = animation_non_linearity;
		t = s*(-2*t*t*t + 3*t*t) + (1-s) * t;
		pos = t * pos_target + (1-t) * pos_pre_animation;
	}
	out_changed.notify();
}

bool ViewPort::needs_update() {
	return (animation_time >= 0);
}

Range ViewPort::range() const {
	return Range(pos, area.width() / pixels_per_sample);
}

void ViewPort::set_range(const Range &r) {
	pos = r.offset;
	pixels_per_sample = (double)area.width() / (double)r.length;
}

void ViewPort::make_sample_visible(int sample, int samples_ahead) {
	double x = sample2screen(sample);
	float border = dsample2screen(samples_ahead);
	float dx = area.width() * BorderFactor;
	float dxr = min(area.width() * BorderFactorRight, dx + border);

	if (samples_ahead > 0) {
		// playback: always jump until the cursor is on the left border
		if ((x > area.x2 - dxr) or (x < area.x1 + dx))
			set_target(sample - dscreen2sample(dx), 0.7f);
	} else {
		// no playback: minimal jump until the cursor is between the borders
		if (x > area.x2 - dxr)
			set_target(sample - dscreen2sample(area.width() - dxr), 0.7f);
		else if (x < area.x1 + dx)
			set_target(sample - dscreen2sample(dx), 0.7f);
	}
}

rect ViewPort::nice_mapping_area() {
	// mapping target area
	float x0 = area.x1;
	float x1 = area.x2;
	if (x1 - x0 > 800)
		x0 += theme.TRACK_HANDLE_WIDTH;
	float w = x1 - x0;
	x0 += w * BorderFactor;
	x1 -= w * BorderFactor;
	return rect(x0, x1, area.y1, area.y2);
}

void ViewPort::show(const Range &r) {
	rect mr = nice_mapping_area();

	// map r into (x0,x1)
	pixels_per_sample = mr.width() / (double)r.length;
	pos = (double)r.start() - (mr.x1 - area.x1) / pixels_per_sample;
	pos_pre_animation = pos;
	pos_target = pos;
	out_changed.notify();
}

// for scroll bar override
void ViewPort::dirty_jump(float _pos) {
	pos = _pos;
	pos_pre_animation = pos;
	pos_target = pos;
	animation_time = -1;
}

}
