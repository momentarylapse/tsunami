/*
 * ActionLayerDelete.cpp
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#include "ActionTrackLayerDelete.h"
/*
#include "../../Data/Song.h"
#include "../Track/Buffer/ActionTrack__DeleteBuffer.h"
#include "ActionTrackLayer__Delete.h"

ActionLayerDelete::ActionLayerDelete(int _index)
{
	index = _index;
}

void ActionLayerDelete::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	for (Track* t : s->tracks){
		TrackLayer &l = t->layers[index];
		for (int i=l.buffers.num-1; i>=0; i--)
			addSubAction(new ActionTrack__DeleteBuffer(t, index, i), s);
	}

	addSubAction(new ActionLayer__Delete(index), s);
}*/

