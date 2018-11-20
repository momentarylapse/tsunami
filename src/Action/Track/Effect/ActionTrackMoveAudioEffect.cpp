/*
 * ActionTrackMoveAudioEffect.cpp
 *
 *  Created on: 20.11.2018
 *      Author: michi
 */

#include "ActionTrackMoveAudioEffect.h"
#include "../../../Data/Track.h"

ActionTrackMoveAudioEffect::ActionTrackMoveAudioEffect(Track* t, int _source, int _target)
{
	track = t;
	source = _source;
	target = _target;
}

void* ActionTrackMoveAudioEffect::execute(Data* d)
{
	track->fx.move(source, target);
	track->notify();
	return nullptr;
}

void ActionTrackMoveAudioEffect::undo(Data* d)
{
	track->fx.move(target, source);
	track->notify();
}
