/*
 * ActionTrackLayerMakeTrack.h
 *
 *  Created on: 14.08.2018
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERMAKETRACK_H_
#define SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERMAKETRACK_H_

#include "../../ActionGroup.h"

class TrackLayer;

class ActionTrackLayerMakeTrack : public ActionGroup
{
public:
	ActionTrackLayerMakeTrack(TrackLayer *layer);

	void build(Data *d) override;

	TrackLayer *layer;
};

#endif /* SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERMAKETRACK_H_ */
