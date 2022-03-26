/*
 * ActionTrackLayerDelete.cpp
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#include "ActionTrackLayerDelete.h"

#include "../../../data/Track.h"
#include "../../../data/TrackLayer.h"
#include "../../../data/Song.h"
#include "../buffer/ActionTrack__DeleteBuffer.h"
#include "ActionTrackLayer__Delete.h"
#include "ActionTrackFadeDelete.h"

ActionTrackLayerDelete::ActionTrackLayerDelete(Track *t, int _index) {
	track = t;
	index = _index;
}

void ActionTrackLayerDelete::build(Data *d) {
	TrackLayer *l = track->layers[index].get();
	for (int i=l->buffers.num-1; i>=0; i--)
		add_sub_action(new ActionTrack__DeleteBuffer(l, i), d);

	add_sub_action(new ActionTrackLayer__Delete(track, index), d);
	for (int i=l->fades.num-1; i>=0; i--)
		add_sub_action(new ActionTrackFadeDelete(l, i), d);
}

