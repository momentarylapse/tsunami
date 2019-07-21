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


ActionTrackLayerActivateVersion::ActionTrackLayerActivateVersion(TrackLayer *l, const Range &r, bool _activate) {
	layer = l;
	range = r;
	activate = _activate;
}

void ActionTrackLayerActivateVersion::del_fades_in_range(Data *d) {
	foreachib (auto &f, layer->fades, i) {
		if (range.is_inside(f.position))
			add_sub_action(new ActionTrackFadeDelete(layer, i), d);
	}
}

int ActionTrackLayerActivateVersion::first_fade_after(int pos) {
	foreachi(auto &f, layer->fades, i)
		if (f.position > pos)
			return i;
	return -1;
}

bool ActionTrackLayerActivateVersion::is_active_at(int pos) {
	int i0 = first_fade_after(pos);
	if (i0 > 0)
		return layer->fades[i0 - 1].mode == CrossFade::INWARD;
	return true;
}


void ActionTrackLayerActivateVersion::build(Data *d) {
	bool active_before = is_active_at(range.start());
	bool active_after = is_active_at(range.end());
	del_fades_in_range(d);
	if (activate) {
		if (!active_before)
			add_sub_action(new ActionTrackFadeAdd(layer, range.start(), CrossFade::INWARD, SAMPLES), d);
		if (!active_after)
		add_sub_action(new ActionTrackFadeAdd(layer, range.end(), CrossFade::OUTWARD, SAMPLES), d);
	} else {
		if (active_before)
			add_sub_action(new ActionTrackFadeAdd(layer, range.start(), CrossFade::OUTWARD, SAMPLES), d);
		if (active_after)
			add_sub_action(new ActionTrackFadeAdd(layer, range.end(), CrossFade::INWARD, SAMPLES), d);
	}
}



ActionTrackLayerMarkDominant::ActionTrackLayerMarkDominant(Track *_track, const Array<const TrackLayer*> &_layers, const Range &_range) {
	layers = _layers;
	range = _range;
	track = _track;
}

void ActionTrackLayerMarkDominant::build(Data *d) {
	for (TrackLayer *l: track->layers) {
		bool activate = (layers.find(l) >= 0);
		add_sub_action(new ActionTrackLayerActivateVersion(l, range, activate), d);
	}
}
