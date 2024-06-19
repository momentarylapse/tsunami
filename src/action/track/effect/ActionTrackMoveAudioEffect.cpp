/*
 * ActionTrackMoveAudioEffect.cpp
 *
 *  Created on: 20.11.2018
 *      Author: michi
 */

#include "ActionTrackMoveAudioEffect.h"
#include "../../../data/Track.h"

namespace tsunami {

ActionTrackMoveAudioEffect::ActionTrackMoveAudioEffect(Track* t, int _source, int _target) {
	track = t;
	source = _source;
	target = _target;
}

void* ActionTrackMoveAudioEffect::execute(Data* d) {
	weak(track->fx).move(source, target);
	track->out_changed.notify();
	return nullptr;
}

void ActionTrackMoveAudioEffect::undo(Data* d) {
	weak(track->fx).move(target, source);
	track->out_changed.notify();
}

}
