/*
 * ActionTrackLayerAdd.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackLayerAdd.h"

#include "../../../Data/Song.h"

ActionTrackLayerAdd::ActionTrackLayerAdd(Track *t, int _index, TrackLayer *l)
{
	track = t;
	index = _index;
	layer = l;
}

ActionTrackLayerAdd::~ActionTrackLayerAdd()
{
	if (layer)
		delete layer;
}

void* ActionTrackLayerAdd::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	track->layers.insert(layer, index);
	a->notify(a->MESSAGE_ADD_LAYER);
	layer = NULL;

	return track->layers[index];
}

void ActionTrackLayerAdd::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	layer = track->layers[index];
	track->layers.erase(index);

	a->notify(a->MESSAGE_DELETE_LAYER);
}


