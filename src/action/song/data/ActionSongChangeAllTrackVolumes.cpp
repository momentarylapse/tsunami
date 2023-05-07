/*
 * ActionSongChangeAllTrackVolumes.cpp
 *
 *  Created on: 24.05.2015
 *      Author: michi
 */

#include "ActionSongChangeAllTrackVolumes.h"
#include "../../../data/Song.h"
#include "../../../data/Track.h"
#include "../../../lib/base/iter.h"

ActionSongChangeAllTrackVolumes::ActionSongChangeAllTrackVolumes(Song *s, Track *t_ref, const Array<const Track*> &_tracks, float _volume) {
	track_no = t_ref->get_index();
	new_value = _volume;
	old_value = t_ref->volume;
	for (auto t: weak(t_ref->song->tracks))
		if (_tracks.find(t) >= 0)
			track_volumes.add({t, t->volume});
}

void *ActionSongChangeAllTrackVolumes::execute(Data *d) {
	float factor = new_value / old_value;

	for (auto &tv: track_volumes) {
		tv.track->volume = tv.old_volume * factor;
		tv.track->out_changed.notify();
	}

	return nullptr;
}

void ActionSongChangeAllTrackVolumes::undo(Data *d) {
	for (auto &tv: track_volumes) {
		tv.track->volume = tv.old_volume;
		tv.track->out_changed.notify();
	}
}


bool ActionSongChangeAllTrackVolumes::mergable(Action *a) {
	auto *aa = dynamic_cast<ActionSongChangeAllTrackVolumes*>(a);
	if (!aa)
		return false;
	if (aa->track_no != track_no)
		return false;
	return aa;
}


