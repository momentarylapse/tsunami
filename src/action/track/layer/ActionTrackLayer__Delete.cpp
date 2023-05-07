/*
 * ActionTrackLayer__Delete.cpp
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#include "ActionTrackLayer__Delete.h"

#include "../../../data/Song.h"
#include "../../../data/Track.h"
#include "../../../data/TrackLayer.h"
#include "../../../data/CrossFade.h"
#include <assert.h>

ActionTrackLayer__Delete::ActionTrackLayer__Delete(Track *t, int _index) {
	track = t;
	index = _index;
	layer = t->layers[index];
}

void* ActionTrackLayer__Delete::execute(Data* d) {
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < track->layers.num);

	assert(layer->buffers.num == 0);
	layer->fake_death();
	track->layers.erase(index);

	a->out_delete_layer.notify();

	return nullptr;
}

void ActionTrackLayer__Delete::undo(Data* d) {
	Song *a = dynamic_cast<Song*>(d);

	track->layers.insert(layer, index);

	a->out_add_layer.notify();
}


