/*
 * ActionTrack__DeleteEmpty.cpp
 *
 *  Created on: 09.04.2012
 *      Author: michi
 */

#include "ActionTrack__DeleteEmpty.h"
#include <assert.h>
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/Song.h"

ActionTrack__DeleteEmpty::ActionTrack__DeleteEmpty(shared<Track> _track) {
	index = _track->get_index();
	track = _track;
}

void *ActionTrack__DeleteEmpty::execute(Data *d) {
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0 and index < a->tracks.num);
	Track *t = a->tracks[index].get();
	int num_buf = 0;
	int num_samples = 0;
	for (TrackLayer *l: weak(t->layers)){
		num_buf += l->buffers.num;
		num_samples += l->samples.num;
	}
	assert(num_buf == 0);
	assert(num_samples == 0);

	// TODO: no layers allowed?!?

	// delete
	a->tracks.erase(index);

	// notify outer structures first!
	a->out_delete_layer.notify();
	a->out_delete_track.notify();
	track->layers[0]->fake_death();
	track->fake_death();
	return nullptr;
}



void ActionTrack__DeleteEmpty::undo(Data *d) {
	Song *a = dynamic_cast<Song*>(d);
	a->tracks.insert(track, index);
	a->out_add_track.notify();
	a->out_add_layer.notify();
}


