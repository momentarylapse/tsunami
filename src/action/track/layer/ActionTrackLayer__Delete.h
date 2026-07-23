/*
 * ActionTrackLayer__Delete.h
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYER__DELETE_H_
#define SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYER__DELETE_H_

#include <lib/history/Action.h>

namespace tsunami {

struct Track;
struct TrackLayer;

class ActionTrackLayer__Delete : public history::Action {
public:
	ActionTrackLayer__Delete(Track *t, int index);

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;
private:
	Track *track;
	int index;
	shared<TrackLayer> layer;
};

}

#endif /* SRC_ACTION_TRACK_LAYER_ACTIONTRACKLAYER__DELETE_H_ */
