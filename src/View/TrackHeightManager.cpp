/*
 * TrackHeightManager.cpp
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#include "TrackHeightManager.h"

#include "AudioView.h"
#include "AudioViewTrack.h"
#include "Mode/ViewMode.h"
#include "../Data/Track.h"

TrackHeightManager::TrackHeightManager()
{
	animating = false;
	t = 0;
	dirty = true;
	render_area = r_id;
}



bool TrackHeightManager::check(Song *a)
{
	return true;
}

rect rect_inter(const rect &a, const rect &b, float t)
{
	return rect((1-t) * a.x1 + t * b.x1,
			(1-t) * a.x2 + t * b.x2,
			(1-t) * a.y1 + t * b.y1,
			(1-t) * a.y2 + t * b.y2);
}

float smooth_parametrization(float t)
{
	if (t < 0.5f)
		return 2*t*t;
	return -2*t*t+4*t-1;
}

void set_track_areas_from_layers(AudioView *view)
{
	for (AudioViewTrack *v : view->vtrack){
		bool first = true;

		for (AudioViewLayer *l: view->vlayer){
			if (l->layer->track != v->track)
				continue;

			if (first){
				v->area = l->area;
			}else{
				v->area.y1 = min(v->area.y1, l->area.y1);
				v->area.y2 = max(v->area.y2, l->area.y2);
			}
			first = false;
		}
	}
}

bool TrackHeightManager::update(AudioView *view, Song *a, const rect &r)
{
	// start animation?
	if (dirty){
		plan(view, a, r);
		t = 0;
		animating = true;
		dirty = false;

		for (AudioViewLayer *v : view->vlayer)
			v->area_last = v->area;
	}

	if (render_area != r){
		render_area = r;
		plan(view, a, r);

		// instant change?
		if (!animating){
			for (AudioViewLayer *v : view->vlayer)
				v->area = v->area_target;
		}
	}

	// force instant changes on x-axis
	for (AudioViewLayer *v : view->vlayer){
		v->area.x1 = v->area_target.x1 = v->area_last.x1 = r.x1;
		v->area.x2 = v->area_target.x2 = v->area_last.x2 = r.x2;
	}

	set_track_areas_from_layers(view);

	if (!animating)
		return false;

	// do the animation
	t += 0.07f;
	if (t >= 1){
		t = 1;
		animating = false;
	}
	for (AudioViewLayer *v : view->vlayer)
		v->area = rect_inter(v->area_last, v->area_target, smooth_parametrization(t));

	set_track_areas_from_layers(view);

	return animating;
}

void TrackHeightManager::plan(AudioView *v, Song *a, const rect &r)
{
	v->mode->updateTrackHeights();

	// wanted space
	int h_wish = 0;
	int h_min = 0;
	for (AudioViewLayer *t : v->vlayer){
		h_wish += t->height_wish;
		h_min += t->height_min;
	}

	// available
	int h_available = (int)v->area.height() - v->TIME_SCALE_HEIGHT;
	float f = 1.0f;
	if (h_wish > h_min)
		f = clampf((float)(h_available - h_min) / (float)(h_wish - h_min), 0, 1);

	// distribute
	int y0 = (int)r.y1 + v->TIME_SCALE_HEIGHT;
	foreachi(AudioViewLayer *t, v->vlayer, i){
		float h = t->height_min + (t->height_wish - t->height_min) * f;
		t->area_target = rect(r.x1, r.x2, y0, y0 + h);
		y0 += (int)h;
	}
}

