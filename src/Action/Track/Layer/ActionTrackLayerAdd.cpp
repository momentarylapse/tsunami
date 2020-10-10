/*
 * ActionTrackLayerAdd.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackLayerAdd.h"

#include "../../../Data/Song.h"
#include "../../../Data/Track.h"
#include "../../../Data/TrackLayer.h"

ActionTrackLayerAdd::ActionTrackLayerAdd(Track *t, shared<TrackLayer> l) {
	track = t;
	layer = l;
}

void* ActionTrackLayerAdd::execute(Data* d) {
	Song *a = dynamic_cast<Song*>(d);

	track->layers.add(layer);
	a->notify(a->MESSAGE_ADD_LAYER);

	return layer.get();
}

void ActionTrackLayerAdd::undo(Data* d) {
	Song *a = dynamic_cast<Song*>(d);

	layer->fake_death();
	track->layers.pop();

	a->notify(a->MESSAGE_DELETE_LAYER);
}


