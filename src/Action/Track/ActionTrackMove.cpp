/*
 * ActionTrackMove.cpp
 *
 *  Created on: 28.08.2017
 *      Author: michi
 */

#include "ActionTrackMove.h"
#include "../../Data/Song.h"

ActionTrackMove::ActionTrackMove(Track *track, int _target)
{
	origin = get_track_index(track);
	target = _target;
}

void *ActionTrackMove::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	s->tracks.move(origin, target);
	s->notify(s->MESSAGE_ADD_TRACK);
	return nullptr;
}

void ActionTrackMove::undo(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	s->tracks.move(target, origin);
	s->notify(s->MESSAGE_ADD_TRACK);
}

