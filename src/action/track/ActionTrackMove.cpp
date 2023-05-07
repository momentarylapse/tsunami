/*
 * ActionTrackMove.cpp
 *
 *  Created on: 28.08.2017
 *      Author: michi
 */

#include "ActionTrackMove.h"
#include "../../data/Song.h"

ActionTrackMove::ActionTrackMove(Track *track, int _target) {
	origin = get_track_index(track);
	target = _target;
}

void *ActionTrackMove::execute(Data *d) {
	Song *s = dynamic_cast<Song*>(d);

	weak(s->tracks).move(origin, target);
	s->out_add_track.notify();
	s->out_add_layer.notify();
	return nullptr;
}

void ActionTrackMove::undo(Data *d) {
	Song *s = dynamic_cast<Song*>(d);

	weak(s->tracks).move(target, origin);
	s->out_add_track.notify();
	s->out_add_layer.notify();
}

