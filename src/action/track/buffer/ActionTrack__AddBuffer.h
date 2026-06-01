/*
 * ActionTrack__AddBuffer.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__ADDBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__ADDBUFFER_H_

#include <lib/history/Action.h>
#include "../../../data/Track.h"

namespace tsunami {

// TODO: move to layer

class TrackLayer;

class ActionTrack__AddBuffer : public history::Action {
public:
	ActionTrack__AddBuffer(TrackLayer *l, int index, const Range &r);

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	TrackLayer *layer;
	int index;
	Range range;
};

}

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__ADDBUFFER_H_ */
