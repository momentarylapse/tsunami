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
	layer = t->layers[index];
}

ActionTrackLayer__Delete::~ActionTrackLayer__Delete()
{
	if (layer)
		delete layer;
}

void* ActionTrackLayer__Delete::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < track->layers.num);

	layer = track->layers[index];
	assert(layer->buffers.num == 0);
	track->layers.erase(index);

	a->notify(a->MESSAGE_DELETE_LAYER);

	return NULL;
}

void ActionTrackLayer__Delete::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	track->layers.insert(layer, index);
	layer = NULL;

	a->notify(a->MESSAGE_ADD_LAYER);
}


