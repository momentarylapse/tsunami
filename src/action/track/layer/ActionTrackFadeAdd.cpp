/*
 * ActionTrackFadeAdd.cpp
 *
 *  Created on: 05.09.2018
 *      Author: michi
 */

#include "ActionTrackFadeAdd.h"
#include "../../../data/TrackLayer.h"


ActionTrackFadeAdd::ActionTrackFadeAdd(TrackLayer* l, int position, CrossFade::Mode mode, int samples) {
	layer = l;
	fade = {position, mode, samples};
	index = 0;
}

void* ActionTrackFadeAdd::execute(Data* d) {
	for (int i=0; i<layer->fades.num; i++)
		if (layer->fades[i].position < fade.position){
			index = i + 1;
		}
	layer->fades.insert(fade, index);

	return nullptr;
}

void ActionTrackFadeAdd::undo(Data* d) {
	layer->fades.erase(index);
}
