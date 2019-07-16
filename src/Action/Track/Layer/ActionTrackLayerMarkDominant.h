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

class TrackLayer;
class Range;

class ActionTrackLayerMarkDominant : public ActionGroup {
public:
	ActionTrackLayerMarkDominant(TrackLayer *layer, const Range &range, bool exclusive);

	void build(Data *d) override;

	TrackLayer *layer;
	Range range;
	bool exclusive;
	
	void del_fades_in_range(TrackLayer *l, const Range &r, Data *d);
	int first_fade_after(TrackLayer *l, int pos);
	bool is_active_at(TrackLayer *l, int pos);
	void set_active(TrackLayer *l, const Range &r, Data *d);
	void set_inactive(TrackLayer *l, const Range &r, Data *d);
};

#endif /* SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERMARKDOMINANT_H_ */
