/*
 * ActionTrackMoveMidiEffect.cpp
 *
 *  Created on: 26.05.2019
 *      Author: michi
 */

#include "../../../data/Track.h"
#include "ActionTrackMoveMidiEffect.h"

namespace tsunami {

ActionTrackMoveMidiEffect::ActionTrackMoveMidiEffect(Track* t, int _source, int _target) {
	track = t;
	source = _source;
	target = _target;
}

void* ActionTrackMoveMidiEffect::execute(Data* d) {
	weak(track->midi_fx).move(source, target);
	track->out_changed.notify();
	return nullptr;
}

void ActionTrackMoveMidiEffect::undo(Data* d) {
	weak(track->midi_fx).move(target, source);
	track->out_changed.notify();
}

}
