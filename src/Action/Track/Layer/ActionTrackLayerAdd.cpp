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

ActionTrackLayerAdd::ActionTrackLayerAdd(Track *t, TrackLayer *l) {
	track = t;
	layer = l;
}

void* ActionTrackLayerAdd::execute(Data* d) {
	Song *a = dynamic_cast<Song*>(d);

	track->layers.add(layer);
	a->notify(a->MESSAGE_ADD_LAYER);
	layer = nullptr;

	return track->layers.back();
}

void ActionTrackLayerAdd::undo(Data* d) {
	Song *a = dynamic_cast<Song*>(d);

	layer = track->layers.back();
	layer->fake_death();
	track->layers.pop();

	a->notify(a->MESSAGE_DELETE_LAYER);
}


