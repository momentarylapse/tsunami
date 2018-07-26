/*
 * ActionTrackDeleteEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include <assert.h>
#include "ActionTrackDeleteAudioEffect.h"
#include "../../../Data/Track.h"
#include "../../../Data/Song.h"
#include "../../../Module/Audio/AudioEffect.h"

ActionTrackDeleteEffect::ActionTrackDeleteEffect(Track *t, int _index)
{
	track = t;
	index = _index;
	effect = nullptr;
}

ActionTrackDeleteEffect::~ActionTrackDeleteEffect()
{
	if (effect)
		delete effect;
}

void *ActionTrackDeleteEffect::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);

	if (track >= 0){
		assert(index < track->fx.num);

		effect = track->fx[index];
		effect->Observable::notify(effect->MESSAGE_DELETE);
		track->fx.erase(index);
		track->notify(track->MESSAGE_DELETE_EFFECT);
	}else{
		assert(index < a->fx.num);

		effect = a->fx[index];
		effect->Observable::notify(effect->MESSAGE_DELETE);
		a->fx.erase(index);
		a->notify(a->MESSAGE_DELETE_EFFECT);
	}

	return nullptr;
}

void ActionTrackDeleteEffect::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);

	if (track >= 0){
		assert(index <= track->fx.num);

		track->fx.insert(effect, index);
		track->notify(track->MESSAGE_ADD_EFFECT);
	}else{
		assert(index < a->fx.num);

		a->fx.insert(effect, index);
		a->notify(a->MESSAGE_ADD_EFFECT);
	}
	effect = nullptr;
}

