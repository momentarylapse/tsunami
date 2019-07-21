/*
 * ActionTrackLayerMarkDominant.h
 *
 *  Created on: 04.09.2018
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERMARKDOMINANT_H_
#define SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERMARKDOMINANT_H_

#include "../../ActionGroup.h"
#include "../../../Data/Range.h"

class Track;
class TrackLayer;
class Range;

class ActionTrackLayerActivateVersion : public ActionGroup {
public:
	ActionTrackLayerActivateVersion(TrackLayer *layer, const Range &r, bool activate);

	void build(Data *d) override;

	TrackLayer *layer;
	Range range;
	bool activate;

	void del_fades_in_range(const Range &r, Data *d);
	bool is_active_at(int pos);
};

class ActionTrackLayerMarkDominant : public ActionGroup {
public:
	ActionTrackLayerMarkDominant(Track *track, const Array<const TrackLayer*> &layers, const Range &range);

	void build(Data *d) override;

	Track *track;
	Array<const TrackLayer*> layers;
	Range range;

	void set_active(TrackLayer *l, const Range &r, Data *d);
	void set_inactive(TrackLayer *l, const Range &r, Data *d);
};

#endif /* SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERMARKDOMINANT_H_ */
