/*
 * ActionTrackDeleteMarker.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "ActionTrackDeleteMarker.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionTrackDeleteMarker::ActionTrackDeleteMarker(Track *t, int _index)
{
	track = t;
	index = _index;
	marker = nullptr;
}

void *ActionTrackDeleteMarker::execute(Data *d)
{
	assert(index >= 0);
	assert(index < track->markers.num);

	marker = track->markers[index];
	track->markers.erase(index);

	return nullptr;
}

void ActionTrackDeleteMarker::undo(Data *d)
{
	track->markers.insert(marker, index);
}

