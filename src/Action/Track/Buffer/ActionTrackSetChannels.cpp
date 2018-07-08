/*
 * ActionTrackSetChannels.cpp
 *
 *  Created on: Jul 8, 2018
 *      Author: michi
 */

#include "ActionTrackSetChannels.h"
#include "../../../Data/Track.h"
#include "ActionTrack__BufferSetChannels.h"

class ActionTrack__SetChannel : public Action
{
public:
	ActionTrack__SetChannel(Track *t, int _channels)
	{ track = t; channels = _channels; }
	void *execute(Data *d) override
	{
		std::swap(track->channels, channels);
		for (TrackLayer *l: track->layers)
			l->channels = track->channels;
		track->song->notify(Song::MESSAGE_CHANGE_CHANNELS);
		return NULL;
	}
	void undo(Data *d) override
	{ execute(d); }
private:
	Track *track;
	int channels;
};

ActionTrackSetChannels::ActionTrackSetChannels(Track *t, int _channels)
{
	track = t;
	channels = _channels;
}

void ActionTrackSetChannels::build(Data *d)
{
	for (TrackLayer *l: track->layers)
		for (int i=0; i<l->buffers.num; i++)
			addSubAction(new ActionTrack__BufferSetChannels(l, i, channels), d);
	addSubAction(new ActionTrack__SetChannel(track, channels), d);
}

