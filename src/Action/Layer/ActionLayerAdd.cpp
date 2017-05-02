/*
 * ActionLayerAdd.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionLayerAdd.h"

#include "../../Data/Song.h"

ActionLayerAdd::ActionLayerAdd(const string &_name, int _index)
{
	name = _name;
	index = _index;
}

void* ActionLayerAdd::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	a->layer_names.insert(name, index);

	TrackLayer new_layer;
	for (Track *t: a->tracks)
		t->layers.insert(new_layer, index);
	a->notify(a->MESSAGE_ADD_LAYER);

	return NULL;
}

void ActionLayerAdd::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->layer_names.erase(index);

	for (Track *t: a->tracks)
		t->layers.erase(index);

	a->notify(a->MESSAGE_DELETE_LAYER);
}


