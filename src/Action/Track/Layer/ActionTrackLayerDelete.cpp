/*
 * ActionTrackLayerDelete.cpp
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#include "ActionTrackLayerDelete.h"

#include "../../../Data/Track.h"
#include "../Buffer/ActionTrack__DeleteBuffer.h"
#include "ActionTrackLayer__Delete.h"

ActionTrackLayerDelete::ActionTrackLayerDelete(Track *t, int _index)
{
	track = t;
	index = _index;
}

void ActionTrackLayerDelete::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	TrackLayer *l = track->layers[index];
	for (int i=l->buffers.num-1; i>=0; i--)
		addSubAction(new ActionTrack__DeleteBuffer(l, i), s);

	addSubAction(new ActionTrackLayer__Delete(track, index), s);
}

