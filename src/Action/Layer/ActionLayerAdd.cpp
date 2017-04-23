/*
 * ActionLayerAdd.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionLayerAdd.h"

#include "../../Data/Song.h"

ActionLayerAdd::ActionLayerAdd(const string &_name)
{
	name = _name;
}

void* ActionLayerAdd::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	a->layer_names.add(name);

	TrackLayer new_layer;
	for (Track *t : a->tracks)
		t->layers.add(new_layer);
	a->notify(a->MESSAGE_ADD_LAYER);

	return NULL;
}

void ActionLayerAdd::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->layer_names.pop();

	for (Track *t : a->tracks)
		t->layers.pop();

	a->notify(a->MESSAGE_DELETE_LAYER);
}


