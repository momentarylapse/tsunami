/*
 * ActionTrackAdd.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "ActionTrackAdd.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include <assert.h>

ActionTrackAdd::ActionTrackAdd(Track *t, int _index) {
	track = t;
	index = _index;
}

ActionTrackAdd::~ActionTrackAdd() {
	if (track)
		delete track;
}

void ActionTrackAdd::undo(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	track = s->tracks[index];
	s->tracks.erase(index);

	// notify outer structures first!
	s->notify(s->MESSAGE_DELETE_LAYER);
	s->notify(s->MESSAGE_DELETE_TRACK);
	track->layers[0]->fake_death();
	track->fake_death();
}



void *ActionTrackAdd::execute(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	track->song = s;

	assert((index >= 0) and (index <= s->tracks.num));

	s->tracks.insert(track, index);

	s->notify(s->MESSAGE_ADD_TRACK);
	s->notify(s->MESSAGE_ADD_LAYER);
	track = nullptr;

	return s->tracks[index];
}


