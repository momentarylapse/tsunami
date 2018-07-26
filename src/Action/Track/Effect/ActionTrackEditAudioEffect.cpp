/*
 * ActionTrackEditEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditEffect.h"
#include "../../../Data/Track.h"
#include <assert.h>

#include "../../../Module/Audio/AudioEffect.h"

ActionTrackEditEffect::ActionTrackEditEffect(AudioEffect *_fx, const string &_old_params)
{
	fx = _fx;
	old_value = _old_params;
	new_value = fx->config_to_string();
}

void *ActionTrackEditEffect::execute(Data *d)
{
	fx->config_from_string(new_value);
	fx->Observable::notify(fx->MESSAGE_CHANGE_BY_ACTION);

	return nullptr;
}

void ActionTrackEditEffect::undo(Data *d)
{
	fx->config_from_string(old_value);
	fx->Observable::notify(fx->MESSAGE_CHANGE_BY_ACTION);
}

bool ActionTrackEditEffect::mergable(Action *a)
{
	ActionTrackEditEffect *aa = dynamic_cast<ActionTrackEditEffect*>(a);
	if (!aa)
		return false;
	return (aa->fx == fx);
}

