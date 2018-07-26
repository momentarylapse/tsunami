/*
 * ActionTrackAddEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackAddEffect.h"
#include "../../../Data/Track.h"
#include "../../../Data/Song.h"
#include "../../../Module/Audio/AudioEffect.h"

ActionTrackAddEffect::ActionTrackAddEffect(Track *t, AudioEffect *_effect)
{
	track = t;
	effect = _effect;
}

ActionTrackAddEffect::~ActionTrackAddEffect()
{
	if (effect)
		delete effect;
}

void *ActionTrackAddEffect::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	if (track){
		track->fx.add(effect);
		track->notify(track->MESSAGE_ADD_EFFECT);
	}else{
		a->fx.add(effect);
		a->notify(a->MESSAGE_ADD_EFFECT);
	}
	effect = nullptr;

	return nullptr;
}

void ActionTrackAddEffect::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	if (track){
		effect = track->fx.back();
		effect->Observable::notify(effect->MESSAGE_DELETE);
		track->fx.pop();
		track->notify(track->MESSAGE_DELETE_EFFECT);
	}else{
		effect = a->fx.back();
		effect->Observable::notify(effect->MESSAGE_DELETE);
		a->fx.pop();
		a->notify(a->MESSAGE_DELETE_EFFECT);
	}
}

