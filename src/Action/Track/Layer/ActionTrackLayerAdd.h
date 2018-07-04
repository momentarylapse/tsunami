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

class ActionTrackLayerAdd : public Action
{
public:
	ActionTrackLayerAdd(Track *t, int index, TrackLayer *l);
	~ActionTrackLayerAdd();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
private:
	Track *track;
	int index;
	TrackLayer *layer;
};

#endif /* SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYERADD_H_ */
