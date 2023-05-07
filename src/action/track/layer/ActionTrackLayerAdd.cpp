/*
 * ActionTrackLayerAdd.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionTrackLayerAdd.h"

#include "../../../data/Song.h"
#include "../../../data/Track.h"
#include "../../../data/TrackLayer.h"

ActionTrackLayerAdd::ActionTrackLayerAdd(Track *t, shared<TrackLayer> l) {
	track = t;
	layer = l;
}

void* ActionTrackLayerAdd::execute(Data* d) {
	Song *a = dynamic_cast<Song*>(d);

	track->layers.add(layer);
	a->out_add_layer.notify();

	return layer.get();
}

void ActionTrackLayerAdd::undo(Data* d) {
	Song *a = dynamic_cast<Song*>(d);

	layer->fake_death();
	track->layers.pop();

	a->out_delete_layer.notify();
}


