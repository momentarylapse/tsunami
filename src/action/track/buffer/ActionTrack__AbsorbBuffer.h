/*
 * ActionTrack__AbsorbBuffer.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__ABSORBBUFFER_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__ABSORBBUFFER_H_

#include <lib/history/Action.h>

namespace tsunami {

struct TrackLayer;

class ActionTrack__AbsorbBuffer : public history::Action {
public:
	ActionTrack__AbsorbBuffer(TrackLayer *l, int dest, int src);

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	TrackLayer *layer;
	int dest, src;
	int dest_old_length;
	int src_offset, src_length;
};

}

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__ABSORBBUFFERBOX_H_ */
