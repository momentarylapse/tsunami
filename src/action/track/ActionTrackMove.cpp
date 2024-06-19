/*
 * ActionTrackMove.cpp
 *
 *  Created on: 28.08.2017
 *      Author: michi
 */

#include "ActionTrackMove.h"
#include "../../data/Song.h"

namespace tsunami {

ActionTrackMove::ActionTrackMove(Track *track, int _target) {
	origin = get_track_index(track);
	target = _target;
}

void *ActionTrackMove::execute(Data *d) {
	Song *s = dynamic_cast<Song*>(d);

	weak(s->tracks).move(origin, target);
	s->out_track_list_changed.notify();
	s->out_layer_list_changed.notify();
	return nullptr;
}

void ActionTrackMove::undo(Data *d) {
	Song *s = dynamic_cast<Song*>(d);

	weak(s->tracks).move(target, origin);
	s->out_track_list_changed.notify();
	s->out_layer_list_changed.notify();
}

}

