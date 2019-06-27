/*
 * ActionTrackLayer__Delete.cpp
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#include "ActionTrackLayer__Delete.h"

#include "../../../Data/Song.h"
#include "../../../Data/Track.h"
#include "../../../Data/TrackLayer.h"
#include "../../../Data/CrossFade.h"
#include <assert.h>

ActionTrackLayer__Delete::ActionTrackLayer__Delete(Track *t, int _index) {
	track = t;
	index = _index;
	layer = t->layers[index];

	foreachi (auto &f, track->fades, i)
		if (f.target >= index and f.target > 0)
			fades_shifted.add(i);
}

ActionTrackLayer__Delete::~ActionTrackLayer__Delete() {
	if (layer)
		delete layer;
}

void* ActionTrackLayer__Delete::execute(Data* d) {
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < track->layers.num);

	for (int i: fades_shifted)
		track->fades[i].target --;

	layer = track->layers[index];
	assert(layer->buffers.num == 0);
	layer->fake_death();
	track->layers.erase(index);

	a->notify(a->MESSAGE_DELETE_LAYER);

	return nullptr;
}

void ActionTrackLayer__Delete::undo(Data* d) {
	Song *a = dynamic_cast<Song*>(d);

	for (int i: fades_shifted)
		track->fades[i].target ++;

	track->layers.insert(layer, index);
	layer = nullptr;

	a->notify(a->MESSAGE_ADD_LAYER);
}


