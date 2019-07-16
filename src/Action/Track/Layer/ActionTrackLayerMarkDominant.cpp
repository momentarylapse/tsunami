/*
 * ActionTrackLayerMarkDominant.cpp
 *
 *  Created on: 04.09.2018
 *      Author: michi
 */

#include "ActionTrackLayerMarkDominant.h"
#include "ActionTrackFadeAdd.h"
#include "ActionTrackFadeDelete.h"
#include "../../../Data/Track.h"
#include "../../../Data/TrackLayer.h"


const int SAMPLES = 2000; // for now, use default value

ActionTrackLayerMarkDominant::ActionTrackLayerMarkDominant(TrackLayer *_layer, const Range &_range, bool _exclusive) {
	layer = _layer;
	range = _range;
	exclusive = _exclusive;
}

void ActionTrackLayerMarkDominant::del_fades_in_range(TrackLayer *l, const Range &r, Data *d) {
	foreachib (auto &f, l->fades, i){
		if (range.is_inside(f.position))
			add_sub_action(new ActionTrackFadeDelete(l, i), d);
	}
}

int ActionTrackLayerMarkDominant::first_fade_after(TrackLayer *l, int pos) {
	foreachi(auto &f, l->fades, i)
		if (f.position > pos)
			return i;
	return -1;
}

bool ActionTrackLayerMarkDominant::is_active_at(TrackLayer *l, int pos) {
	int i0 = first_fade_after(l, pos);
	if (i0 > 0)
		return l->fades[i0 - 1].mode == CrossFade::INWARD;
	return l->is_main();
}

void ActionTrackLayerMarkDominant::set_active(TrackLayer *l, const Range &r, Data *d) {
	bool active_before = is_active_at(l, r.start());
	bool active_after = is_active_at(l, r.end());
	del_fades_in_range(l, r, d);
	if (!active_before)
		add_sub_action(new ActionTrackFadeAdd(l, range.start(), CrossFade::INWARD, SAMPLES), d);
	if (!active_after)
		add_sub_action(new ActionTrackFadeAdd(l, range.end(), CrossFade::OUTWARD, SAMPLES), d);
}

void ActionTrackLayerMarkDominant::set_inactive(TrackLayer *l, const Range &r, Data *d) {
	bool active_before = is_active_at(l, r.start());
	bool active_after = is_active_at(l, r.end());
	del_fades_in_range(l, r, d);
	if (active_before)
		add_sub_action(new ActionTrackFadeAdd(l, range.start(), CrossFade::OUTWARD, SAMPLES), d);
	if (active_after)
		add_sub_action(new ActionTrackFadeAdd(l, range.end(), CrossFade::INWARD, SAMPLES), d);
}

void ActionTrackLayerMarkDominant::build(Data *d) {
	if (exclusive) {
		Track *track = layer->track;
		for (TrackLayer *l: track->layers)
			if (l == layer)
				set_active(l, range, d);
			else
				set_inactive(l, range, d);
	} else {
		set_active(layer, range, d);
	}
}
