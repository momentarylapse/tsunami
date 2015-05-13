/*
 * ActionTrackAddMarker.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "ActionTrackAddMarker.h"
#include "../../../Data/Track.h"

ActionTrackAddMarker::ActionTrackAddMarker(Track *t, int _pos, const string &_text)
{
	track_no = get_track_index(t);
	pos = _pos;
	text = _text;
}

void *ActionTrackAddMarker::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	Track *t = a->get_track(track_no);
	TrackMarker m;
	m.pos = pos;
	m.text = text;
	t->markers.add(m);

	return NULL;
}

void ActionTrackAddMarker::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);
	t->markers.pop();
}

