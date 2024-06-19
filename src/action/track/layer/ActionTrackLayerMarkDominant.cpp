/*
 * ActionTrackLayerMarkDominant.cpp
 *
 *  Created on: 04.09.2018
 *      Author: michi
 */

#include "ActionTrackLayerMarkDominant.h"
#include "ActionTrackFadeAdd.h"
#include "ActionTrackFadeDelete.h"
#include "../../../data/Track.h"
#include "../../../data/TrackLayer.h"

namespace tsunami {

const int SAMPLES = 2000; // for now, use default value


ActionTrackLayerActivateVersion::ActionTrackLayerActivateVersion(TrackLayer *l, const Range &r, bool _activate) {
	layer = l;
	range = r;
	activate = _activate;
}

void ActionTrackLayerActivateVersion::del_fades_in_range(const Range &r, Data *d) {
	foreachib (auto &f, layer->fades, i)
		if (r.overlaps(f.range()))
			add_sub_action(new ActionTrackFadeDelete(layer, i), d);
}

bool ActionTrackLayerActivateVersion::is_active_at(int pos) {
	bool active = true;
	for (auto &f: layer->fades) {
		if (f.range().is_inside(pos)) {
			return true;
		} else if (f.range().end() < pos) {
			active = (f.mode == CrossFade::INWARD);
		}
	}
	return active;
}

int r_dist(const Range &r, int pos) {
	if (pos < r.start())
		return r.start() - pos;
	if (pos > r.end())
		return pos - r.end();
	return 0;
}

Range grow_reasonable(TrackLayer *l, const Range &r) {
	Range rr = r;
	bool dirty = true;
	int dd = 1000;
	while (dirty) {
		dirty = false;
		for (auto &f: l->fades) {
			if (r_dist(f.range(), rr.start()) < dd) {
				rr.set_start(f.range().start() - dd);
				dirty = true;
			}
			if (r_dist(f.range(), rr.end()) < dd) {
				rr.set_end(f.range().end() + dd);
				dirty = true;
			}
		}
	}
	return rr;
}


void ActionTrackLayerActivateVersion::build(Data *d) {
	Range rr = grow_reasonable(layer, range);
	bool active_before = is_active_at(rr.start());
	bool active_after = is_active_at(rr.end());
	del_fades_in_range(rr, d);
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
	for (TrackLayer *l: weak(track->layers)) {
		bool activate = (layers.find(l) >= 0);
		add_sub_action(new ActionTrackLayerActivateVersion(l, range, activate), d);
	}
}

}
