/*
 * ActionTrackLayer__Delete.cpp
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#include "ActionTrackLayer__Delete.h"

#include "../../../Data/Song.h"
#include <assert.h>

ActionTrackLayer__Delete::ActionTrackLayer__Delete(Track *t, int _index)
{
	track = t;
	index = _index;
	type = t->layers[index].type;
}

void* ActionTrackLayer__Delete::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < track->layers.num);

	for (Track *t: a->tracks){
		assert(t->layers[index].buffers.num == 0);
		t->layers.erase(index);
	}

	a->notify(a->MESSAGE_DELETE_LAYER);

	return NULL;
}

void ActionTrackLayer__Delete::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	TrackLayer new_layer;
	new_layer.type = type;
	for (Track *t: a->tracks)
		t->layers.insert(new_layer, index);

	a->notify(a->MESSAGE_ADD_LAYER);
}


