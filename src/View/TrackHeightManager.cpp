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

TrackHeightManager::TrackHeightManager()
{
	animating = false;
	t = 0;
	dirty = true;
	render_area = r_id;
	midi_track = NULL;
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

bool TrackHeightManager::update(AudioView *v, Song *a, const rect &r)
{
	Track *new_midi_track = (v->editingMidi() ? v->cur_track : NULL);


	// start animation?
	if ((dirty) or (midi_track != new_midi_track)){
		plan(v, a, r);
		t = 0;
		animating = true;
		dirty = false;
		midi_track = new_midi_track;

		foreach(AudioViewTrack *v, v->vtrack)
			v->area_last = v->area;
	}

	if (render_area != r){
		render_area = r;
		plan(v, a, r);

		// instant change?
		if (!animating){
			foreach(AudioViewTrack *v, v->vtrack)
				v->area = v->area_target;
		}
	}

	// force instant changes on x-axis
	foreach(AudioViewTrack *v, v->vtrack){
		v->area.x1 = v->area_target.x1 = v->area_last.x1 = r.x1;
		v->area.x2 = v->area_target.x2 = v->area_last.x2 = r.x2;
	}

	if (!animating)
		return false;

	// do the animation
	t += 0.07f;
	if (t >= 1){
		t = 1;
		animating = false;
	}
	foreach(AudioViewTrack *v, v->vtrack)
		v->area = rect_inter(v->area_last, v->area_target, smooth_parametrization(t));

	return animating;
}

void TrackHeightManager::plan(AudioView *v, Song *a, const rect &r)
{
	v->mode->updateTrackHeights();

	// wanted space
	int h_wish = 0;
	int h_min = 0;
	foreach(AudioViewTrack *t, v->vtrack){
		h_wish += t->height_wish;
		h_min += t->height_min;
	}

	// available
	int h_available = v->area.height() - v->TIME_SCALE_HEIGHT;
	float f = 1.0f;
	if (h_wish > h_min)
		f = clampf((float)(h_available - h_min) / (float)(h_wish - h_min), 0, 1);

	// distribute
	int y0 = r.y1 + v->TIME_SCALE_HEIGHT;
	foreachi(AudioViewTrack *t, v->vtrack, i){
		float h = t->height_min + (t->height_wish - t->height_min) * f;
		t->area_target = rect(r.x1, r.x2, y0, y0 + h);
		y0 += h;
	}
}

