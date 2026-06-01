/*
 * ActionTrackFadeDelete.cpp
 *
 *  Created on: 05.09.2018
 *      Author: michi
 */

#include "ActionTrackFadeDelete.h"
#include "../../../data/TrackLayer.h"

namespace tsunami {

ActionTrackFadeDelete::ActionTrackFadeDelete(TrackLayer* l, int _index) {
	layer = l;
	index = _index;
}

void* ActionTrackFadeDelete::execute(history::Data* d) {
	fade = layer->fades[index];
	layer->fades.erase(index);

	return nullptr;
}

void ActionTrackFadeDelete::undo(history::Data* d) {
	layer->fades.insert(fade, index);
}

}
