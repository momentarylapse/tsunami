/*
 * TrackHeightManager.cpp
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#include "TrackHeightManager.h"

#include "AudioView.h"
#include "AudioViewTrack.h"

TrackHeightManager::TrackHeightManager()
{
}



bool TrackHeightManager::check(AudioFile *a)
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

bool TrackHeightManager::update(AudioView *v, AudioFile *a, const rect &r)
{
	Track *new_midi_track = (v->editingMidi() ? v->cur_track : NULL);
	if ((dirty) or (render_area != r) or (midi_track != new_midi_track)){
		plan(v, a, r);
		t = 0;
		animating = true;
		dirty = false;

		foreach(AudioViewTrack *v, v->vtrack)
			v->area_last = v->area;
	}
	midi_track = new_midi_track;
	foreach(AudioViewTrack *v, v->vtrack){
		v->area.x1 = v->area_target.x1 = v->area_last.x1 = r.x1;
		v->area.x2 = v->area_target.x2 = v->area_last.x2 = r.x2;
	}


	if (render_area != r){
		render_area = r;
		foreach(AudioViewTrack *v, v->vtrack)
			v->area = v->area_target;
		animating = false;
		t = 0;
	}

	if (!animating)
		return false;

	t += 0.07f;
	if (t >= 1){
		t = 1;
		animating = false;
	}
	foreach(AudioViewTrack *v, v->vtrack)
		v->area = rect_inter(v->area_last, v->area_target, (t < 0.5f) ? 2*t*t : -2*t*t+4*t-1);

	return animating;
}

void TrackHeightManager::plan(AudioView *v, AudioFile *a, const rect &r)
{
	if (v->editingMidi()){
		float y0 = v->TIME_SCALE_HEIGHT;
		foreachi(AudioViewTrack *t, v->vtrack, i){
			float h = v->TIME_SCALE_HEIGHT;
			if (t->track == v->cur_track)
				h = r.height() - a->tracks.num * v->TIME_SCALE_HEIGHT;
			t->area_target = rect(r.x1, r.x2, y0, y0 + h);
			y0 += h;
		}
		return;
	}
	int n_ch = v->show_mono ? 1 : 2;

	int h_wish = v->TIME_SCALE_HEIGHT;
	int h_fix = v->TIME_SCALE_HEIGHT;
	int n_var = 0;
	foreach(Track *t, a->tracks){
		if (t->type == t->TYPE_AUDIO){
			h_wish += v->MAX_TRACK_CHANNEL_HEIGHT * n_ch;
			n_var += n_ch;
		}else if (t->type == t->TYPE_MIDI){
			h_wish += v->MAX_TRACK_CHANNEL_HEIGHT;
			n_var ++;
		}else{
			h_wish += v->TIME_SCALE_HEIGHT * 2;
			h_fix += v->TIME_SCALE_HEIGHT * 2;
		}
	}

	int y0 = r.y1 + v->TIME_SCALE_HEIGHT;
	int opt_channel_height = v->MAX_TRACK_CHANNEL_HEIGHT;
	if (h_wish > r.height())
		opt_channel_height = (r.height() - h_fix) / n_var;
	foreachi(AudioViewTrack *t, v->vtrack, i){
		float h = v->TIME_SCALE_HEIGHT*2;
		if (t->track->type == Track::TYPE_AUDIO)
			h = opt_channel_height * n_ch;
		else if (t->track->type == Track::TYPE_MIDI)
			h = opt_channel_height;
		t->area_target = rect(r.x1, r.x2, y0, y0 + h);
		y0 += h;
	}
}

