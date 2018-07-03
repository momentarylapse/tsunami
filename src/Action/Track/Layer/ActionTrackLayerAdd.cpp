/*
 * ActionTrackLayerAdd.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackLayerAdd.h"

#include "../../../Data/Song.h"

ActionTrackLayerAdd::ActionTrackLayerAdd(Track *t, int _index, int _type)
{
	track = t;
	index = _index;
	type = _type;
}

void* ActionTrackLayerAdd::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	TrackLayer new_layer;
	new_layer.type = type;
	track->layers.insert(new_layer, index);
	a->notify(a->MESSAGE_ADD_LAYER);

	return NULL;
}

void ActionTrackLayerAdd::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	track->layers.erase(index);

	a->notify(a->MESSAGE_DELETE_LAYER);
}


