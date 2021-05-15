/*
 * ActionTrackDelete.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionTrackDelete.h"
#include "Sample/ActionTrackDeleteSample.h"
#include "ActionTrack__DeleteEmpty.h"
#include <assert.h>
#include "Buffer/ActionTrack__DeleteBuffer.h"
#include "Data/ActionTrackSetTarget.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/SampleRef.h"

ActionTrackDelete::ActionTrackDelete(Track *_track) {
	track = _track;
}

void ActionTrackDelete::build(Data *d) {

	// delete buffers
	for (auto *l: weak(track->layers)) {
		for (int i=l->buffers.num-1; i>=0; i--)
			add_sub_action(new ActionTrack__DeleteBuffer(l, i), d);

		// delete samples
		for (int i=l->samples.num-1; i>=0; i--)
			add_sub_action(new ActionTrackDeleteSample(l->samples[i]), d);
	}

	for (auto t: weak(track->song->tracks))
		if (t->send_target == track)
			add_sub_action(new ActionTrackSetTarget(t, nullptr), d);

	// delete the track itself
	add_sub_action(new ActionTrack__DeleteEmpty(track), d);
}
