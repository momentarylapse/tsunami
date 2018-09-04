/*
 * ActionTrackLayerMarkDominant.h
 *
 *  Created on: 04.09.2018
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERMARKDOMINANT_H_
#define SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERMARKDOMINANT_H_

#include "../../Action.h"

class Track;
class TrackLayer;
class Range;

class ActionTrackLayerMarkDominant : public Action
{
public:
	ActionTrackLayerMarkDominant(TrackLayer *layer, const Range &range);

	void *execute(Data *d) override;
	void undo(Data *d) override;

	struct Operation
	{
		int index;
		int target;
		int position;
		int samples;
	};
	Array<Operation> inserts, deletes;
	Track *track;
};

#endif /* SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERMARKDOMINANT_H_ */
