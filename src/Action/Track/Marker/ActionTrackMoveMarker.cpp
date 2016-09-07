/*
 * ActionTrackMoveMarker.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "ActionTrackMoveMarker.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionTrackMoveMarker::ActionTrackMoveMarker(Track *t, int _index, int _pos)
{
	track_no = get_track_index(t);
	index = _index;
	pos = _pos;
}

void *ActionTrackMoveMarker::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	Track *t = a->get_track(track_no);

	assert(index >= 0);
	assert(index < t->markers.num);

	int temp = pos;
	pos = t->markers[index]->pos;
	t->markers[index]->pos = temp;

	return NULL;
}

void ActionTrackMoveMarker::undo(Data *d)
{
	execute(d);
}

