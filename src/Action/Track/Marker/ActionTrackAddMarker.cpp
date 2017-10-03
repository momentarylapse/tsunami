/*
 * ActionTrackAddMarker.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "ActionTrackAddMarker.h"
#include "../../../Data/Track.h"

ActionTrackAddMarker::ActionTrackAddMarker(Track *t, const Range &range, const string &text)
{
	track_no = get_track_index(t);
	marker = new TrackMarker;
	marker->range = range;
	marker->text = text;
}

void *ActionTrackAddMarker::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	Track *t = a->get_track(track_no);
	t->markers.add(marker);

	return marker;
}

void ActionTrackAddMarker::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);
	t->markers.pop();
}

