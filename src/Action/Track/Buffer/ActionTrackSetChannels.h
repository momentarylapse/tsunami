/*
 * ActionTrackSetChannels.h
 *
 *  Created on: Jul 8, 2018
 *      Author: michi
 */

#ifndef SRC_ACTION_TRACK_BUFFER_ACTIONTRACKSETCHANNELS_H_
#define SRC_ACTION_TRACK_BUFFER_ACTIONTRACKSETCHANNELS_H_

#include "../../ActionGroup.h"

class Track;

class ActionTrackSetChannels : public ActionGroup {
public:
	ActionTrackSetChannels(Track *t, int channels);
	void build(Data *d) override;

private:
	Track *track;
	int channels;
};

#endif /* SRC_ACTION_TRACK_BUFFER_ACTIONTRACKSETCHANNELS_H_ */
