/*
 * ActionTrack__DeleteBuffer.h
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__DELETEBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__DELETEBUFFER_H_

#include <lib/history/Action.h>
#include "../../../data/audio/AudioBuffer.h"

namespace tsunami {

class TrackLayer;

class ActionTrack__DeleteBuffer : public history::Action {
public:
	ActionTrack__DeleteBuffer(TrackLayer *l, int _index);

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	TrackLayer *layer;
	int index;
	AudioBuffer buf;
};

}

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__DELETEBUFFER_H_ */
