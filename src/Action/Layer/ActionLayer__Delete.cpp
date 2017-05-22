/*
 * ActionLayer__Delete.cpp
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#include "../../Data/Song.h"
#include <assert.h>
#include "ActionLayer__Delete.h"

ActionLayer__Delete::ActionLayer__Delete(int _index)
{
	index = _index;
	layer = NULL;
}

ActionLayer__Delete::~ActionLayer__Delete()
{
	if (layer)
		delete((Song::Layer*)layer);
}

void* ActionLayer__Delete::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < a->layers.num);

	layer = a->layers[index];
	a->layers.erase(index);

	for (Track *t: a->tracks){
		assert(t->layers[index].buffers.num == 0);
		t->layers.erase(index);
	}

	a->notify(a->MESSAGE_DELETE_LAYER);

	return NULL;
}

void ActionLayer__Delete::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->layers.insert((Song::Layer*)layer, index);
	layer = NULL;

	TrackLayer new_layer;
	for (Track *t: a->tracks)
		t->layers.insert(new_layer, index);

	a->notify(a->MESSAGE_ADD_LAYER);
}


