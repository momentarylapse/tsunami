/*
 * ActionTrackSetChannels.cpp
 *
 *  Created on: Jul 8, 2018
 *      Author: michi
 */

#include "ActionTrackSetChannels.h"
#include "../../../data/Track.h"
#include "../../../data/TrackLayer.h"
#include "../../../data/Song.h"
#include "ActionTrack__BufferSetChannels.h"

class ActionTrack__SetChannel : public Action  {
public:
	ActionTrack__SetChannel(Track *t, int _channels)
	{ track = t; channels = _channels; }
	void *execute(Data *d) override {
		std::swap(track->channels, channels);
		for (auto l: weak(track->layers))
			l->channels = track->channels;
		track->song->out_channels_changed.notify();
		return nullptr;
	}
	void undo(Data *d) override
	{ execute(d); }
private:
	Track *track;
	int channels;
};

ActionTrackSetChannels::ActionTrackSetChannels(Track *t, int _channels) {
	track = t;
	channels = _channels;
}

void ActionTrackSetChannels::build(Data *d) {
	for (auto l: weak(track->layers))
		for (int i=0; i<l->buffers.num; i++)
			add_sub_action(new ActionTrack__BufferSetChannels(l, i, channels), d);
	add_sub_action(new ActionTrack__SetChannel(track, channels), d);
}

