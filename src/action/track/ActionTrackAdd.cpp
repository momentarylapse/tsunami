/*
 * ActionTrackAdd.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionTrackAdd.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include <assert.h>

namespace tsunami {

ActionTrackAdd::ActionTrackAdd(Track *t, int _index) {
	track = t;
	index = _index;
}

void ActionTrackAdd::undo(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	s->tracks.erase(index);

	// notify outer structures first!
	s->out_layer_list_changed.notify();
	s->out_track_list_changed.notify();
	track->layers[0]->fake_death();
	track->fake_death();
}



void *ActionTrackAdd::execute(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	track->song = s;

	assert((index >= 0) and (index <= s->tracks.num));

	s->tracks.insert(track, index);

	s->out_track_list_changed.notify();
	s->out_layer_list_changed.notify();

	return track.get();
}

}

