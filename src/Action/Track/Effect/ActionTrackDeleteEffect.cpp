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
	assert(index >= 0);

	assert(index < track->fx.num);

	effect = track->fx[index];
	effect->Observable::notify(effect->MESSAGE_DELETE);
	track->fx.erase(index);
	track->notify(track->MESSAGE_DELETE_EFFECT);

	return nullptr;
}

void ActionTrackDeleteEffect::undo(Data *d)
{
	assert(index >= 0);

	assert(index <= track->fx.num);

	track->fx.insert(effect, index);
	track->notify(track->MESSAGE_ADD_EFFECT);
	effect = nullptr;
}

