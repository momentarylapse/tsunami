/*
 * ActionLayerAdd.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionLayerAdd.h"

#include "../../Data/Song.h"

ActionLayerAdd::ActionLayerAdd(const string &name, int _index)
{
	layer = new Song::Layer(name);
	index = _index;
}

ActionLayerAdd::~ActionLayerAdd()
{
	if (layer)
		delete((Song::Layer*)layer);
}

void* ActionLayerAdd::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	a->layers.insert((Song::Layer*)layer, index);
	void *r = layer;
	layer = NULL;

	TrackLayer new_layer;
	for (Track *t: a->tracks)
		t->layers.insert(new_layer, index);
	a->notify(a->MESSAGE_ADD_LAYER);

	return r;
}

void ActionLayerAdd::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->layers.erase(index);

	for (Track *t: a->tracks)
		t->layers.erase(index);

	a->notify(a->MESSAGE_DELETE_LAYER);
}


