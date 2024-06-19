/*
 * ActionTrack__BufferSetChannels.h
 *
 *  Created on: Jul 8, 2018
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__BUFFERSETCHANNELS_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__BUFFERSETCHANNELS_H_

#include "../../Action.h"

namespace tsunami {

class TrackLayer;

class ActionTrack__BufferSetChannels : public Action {
public:
	ActionTrack__BufferSetChannels(TrackLayer *layer, int index, int channels);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	TrackLayer *layer;
	int index;
	int channels;
	Array<float> temp;
};

}

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACK__BUFFERSETCHANNELS_H_ */
