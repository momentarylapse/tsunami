/*
 * ActionTrackLayer__Delete.h
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYER__DELETE_H_
#define SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYER__DELETE_H_

#include "../../Action.h"

namespace tsunami {

class Track;
class TrackLayer;

class ActionTrackLayer__Delete : public Action {
public:
	ActionTrackLayer__Delete(Track *t, int index);

	void *execute(Data *d) override;
	void undo(Data *d) override;
private:
	Track *track;
	int index;
	shared<TrackLayer> layer;
};

}

#endif /* SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYER__DELETE_H_ */
