/*
 * ActionTrackLayerAdd.h
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERADD_H_
#define SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERADD_H_

#include "../../Action.h"

class Track;
class TrackLayer;

class ActionTrackLayerAdd : public Action {
public:
	ActionTrackLayerAdd(Track *t, shared<TrackLayer> l);

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Track *track;
	shared<TrackLayer> layer;
};

#endif /* SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERADD_H_ */
