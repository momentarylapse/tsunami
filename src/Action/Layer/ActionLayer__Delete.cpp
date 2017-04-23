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
}

void* ActionLayer__Delete::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < a->layer_names.num);

	name = a->layer_names[index];
	a->layer_names.erase(index);

	for (Track *t : a->tracks){
		assert(t->layers[index].buffers.num == 0);
		t->layers.erase(index);
	}

	a->notify(a->MESSAGE_DELETE_LAYER);

	return NULL;
}

void ActionLayer__Delete::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->layer_names.insert(name, index);

	TrackLayer new_layer;
	for (Track *t : a->tracks)
		t->layers.insert(new_layer, index);

	a->notify(a->MESSAGE_ADD_LAYER);
}


