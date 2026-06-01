/*
 * ActionSongChangeAllTrackVolumes.h
 *
 *  Created on: 24.05.2015
 *      Author: michi
 */

#pragma once

#include <lib/history/MergableAction.h>

namespace tsunami {

class Song;
class Track;

class ActionSongChangeAllTrackVolumes : public history::MergableValueAction<float> {
public:
	ActionSongChangeAllTrackVolumes(Song *s, Track *t_ref, const Array<const Track*> &tracks, float volume);

	string name() const override { return ":##:set volume"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	bool mergable(Action *a) override;

	int track_no;

	struct TrackVolume {
		Track *track;
		float old_volume;
	};
	Array<TrackVolume> track_volumes;
};

}

