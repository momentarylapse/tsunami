/*
 * ActionTrackAddMarker.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "ActionTrackAddMarker.h"
#include "../../../Data/Track.h"
#include "../../../Data/TrackMarker.h"

ActionTrackAddMarker::ActionTrackAddMarker(Track *t, const Range &range, const string &text)
{
	track = t;
	marker = new TrackMarker;
	marker->range = range;
	marker->text = text;
}

void *ActionTrackAddMarker::execute(Data *d)
{
	track->markers.add(marker);

	return marker;
}

void ActionTrackAddMarker::undo(Data *d)
{
	track->markers.pop();
}

