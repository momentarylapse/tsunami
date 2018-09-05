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

class ActionTrackLayerMarkDominant : public ActionGroup
{
public:
	ActionTrackLayerMarkDominant(TrackLayer *layer, const Range &range);

	void build(Data *d) override;

	TrackLayer *layer;
	Range range;
};

#endif /* SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERMARKDOMINANT_H_ */
