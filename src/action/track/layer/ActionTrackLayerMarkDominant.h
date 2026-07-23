/*
 * ActionTrackLayerMarkDominant.h
 *
 *  Created on: 04.09.2018
 *      Author: michi
 */

#pragma once

#include <lib/history/ActionGroup.h>
#include "../../../data/Range.h"

namespace tsunami {

struct Track;
struct TrackLayer;
struct Range;

class ActionTrackLayerActivateVersion : public history::ActionGroup {
public:
	ActionTrackLayerActivateVersion(TrackLayer *layer, const Range &r, bool activate);

	string name() const override { return ":##:dominant"; }

	void* compose(history::Data* d) override;

	TrackLayer *layer;
	Range range;
	bool activate;

	void del_fades_in_range(const Range &r, history::Data *d);
	bool is_active_at(int pos);
};

class ActionTrackLayerMarkDominant : public history::ActionGroup {
public:
	ActionTrackLayerMarkDominant(Track *track, const Array<const TrackLayer*> &layers, const Range &range);

	string name() const override { return ":##:dominant"; }

	void* compose(history::Data* d) override;

	Track *track;
	Array<const TrackLayer*> layers;
	Range range;

	void set_active(TrackLayer *l, const Range &r, history::Data *d);
	void set_inactive(TrackLayer *l, const Range &r, history::Data *d);
};

}
