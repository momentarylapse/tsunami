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
	track_no = get_track_index(t);
	index = _index;
}

void *ActionTrackDeleteMarker::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	Track *t = a->get_track(track_no);
	assert(index >= 0);
	assert(index < t->markers.num);

	pos = t->markers[index].pos;
	text = t->markers[index].text;
	t->markers.erase(index);

	return NULL;
}

void ActionTrackDeleteMarker::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	TrackMarker m;
	m.pos = pos;
	m.text = text;
	t->markers.insert(m, index);
}

