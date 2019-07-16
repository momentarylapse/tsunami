/*
 * ActionTrackFadeDelete.cpp
 *
 *  Created on: 05.09.2018
 *      Author: michi
 */

#include "ActionTrackFadeDelete.h"
#include "../../../Data/TrackLayer.h"


ActionTrackFadeDelete::ActionTrackFadeDelete(TrackLayer* l, int _index) {
	layer = l;
	index = _index;
}

void* ActionTrackFadeDelete::execute(Data* d) {
	fade = layer->fades[index];
	layer->fades.erase(index);

	return nullptr;
}

void ActionTrackFadeDelete::undo(Data* d) {
	layer->fades.insert(fade, index);
}
