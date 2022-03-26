/*
 * ActionTrackMoveMidiEffect.cpp
 *
 *  Created on: 26.05.2019
 *      Author: michi
 */

#include "../../../data/Track.h"
#include "ActionTrackMoveMidiEffect.h"

ActionTrackMoveMidiEffect::ActionTrackMoveMidiEffect(Track* t, int _source, int _target) {
	track = t;
	source = _source;
	target = _target;
}

void* ActionTrackMoveMidiEffect::execute(Data* d) {
	weak(track->midi_fx).move(source, target);
	track->notify();
	return nullptr;
}

void ActionTrackMoveMidiEffect::undo(Data* d) {
	weak(track->midi_fx).move(target, source);
	track->notify();
}
