/*
 * TrackHeightManager.cpp
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#include "TrackHeightManager.h"

#include "AudioView.h"
#include "Mode/ViewMode.h"
#include "../Data/Track.h"
#include "../Data/TrackLayer.h"
#include "Graph/AudioViewLayer.h"
#include "Graph/AudioViewTrack.h"
#include "Helper/Graph/ScrollBar.h"

TrackHeightManager::TrackHeightManager() {
	animating = false;
	t = 0;
	_dirty = true;
	render_area = rect::ID;
}


void TrackHeightManager::set_dirty() {
	_dirty = true;
}

bool TrackHeightManager::check(Song *a) {
	return true;
}

rect rect_inter(const rect &a, const rect &b, float t) {
	return rect((1-t) * a.x1 + t * b.x1,
			(1-t) * a.x2 + t * b.x2,
			(1-t) * a.y1 + t * b.y1,
			(1-t) * a.y2 + t * b.y2);
}

float smooth_parametrization(float t) {
	if (t < 0.5f)
		return 2*t*t;
	return -2*t*t+4*t-1;
}

void set_track_areas_from_layers(AudioView *view) {
	for (auto *v: view->vtracks) {
		bool first = true;

		for (auto *l: view->vlayers) {
			if (l->layer->track != v->track)
				continue;

			if (first) {
				v->area = l->area;
			} else {
				v->area.y1 = min(v->area.y1, l->area.y1);
				v->area.y2 = max(v->area.y2, l->area.y2);
			}
			first = false;
		}
	}
}

bool TrackHeightManager::update(AudioView *view, Song *a, const rect &r) {
	// start animation?
	if (_dirty) {
		plan(view, a, r);
		t = 0;
		animating = true;
		_dirty = false;

		for (auto *v: view->vlayers)
			v->area_last = v->area;
	}

	if (render_area != r) {
		render_area = r;
		plan(view, a, r);

		// instant change?
		if (!animating) {
			for (auto *v: view->vlayers)
				v->area = v->area_target;
		}
	}

	// force instant changes on x-axis
	for (auto *v: view->vlayers) {
		v->area.x1 = v->area_target.x1 = v->area_last.x1 = r.x1;
		v->area.x2 = v->area_target.x2 = v->area_last.x2 = r.x2;
	}

	set_track_areas_from_layers(view);

	if (!animating)
		return false;

	// do the animation
	t += 0.07f;
	if (t >= 1) {
		t = 1;
		animating = false;
	}
	for (auto *v: view->vlayers) {
		v->area = rect_inter(v->area_last, v->area_target, smooth_parametrization(t));
		v->align.w = v->area.width();
		v->align.h = v->area.height();
	}

	set_track_areas_from_layers(view);

	return animating;
}

void TrackHeightManager::update_immediately(AudioView *view, Song *a, const rect &r) {
	plan(view, a, r);
	animating = false;
	_dirty = false;

	for (auto *v : view->vlayers)
		v->area = v->area_target;

	set_track_areas_from_layers(view);
}

void TrackHeightManager::plan(AudioView *v, Song *__a, const rect &r) {
	for (auto *l: v->vlayers) {
		l->height = v->mode->layer_suggested_height(l) * v->cam.scale_y;
		if (l->hidden)
			l->height = 0;
	}

	// wanted space
	float h = 0;
	for (auto *l: v->vlayers)
		h += l->height;

	// available
	v->scroll_bar_y->set_view_size(r.height());
	v->scroll_bar_y->set_content(0, h);


	// distribute
	float y0 = r.y1 - v->scroll_bar_y->get_view_offset();
	for (auto *l: v->vlayers) {
		l->area_target = rect(r.x1, r.x2, y0, y0 + l->height);
		y0 += l->height;
	}
}

