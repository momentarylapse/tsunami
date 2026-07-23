/*
 * ActionTrack__SplitBuffer.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__SPLITBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__SPLITBUFFER_H_

#include <lib/history/Action.h>

namespace tsunami {

struct TrackLayer;

class ActionTrack__SplitBuffer : public history::Action {
public:
	ActionTrack__SplitBuffer(shared<TrackLayer> l, int _index, int _offset);

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	shared<TrackLayer> layer;
	int index;
	int offset;
};

}

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__SPLITBUFFER_H_ */
