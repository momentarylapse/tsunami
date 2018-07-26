/*
 * ActionTrackToggleEffectEnabled.cpp
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#include "ActionTrackToggleEffectEnabled.h"
#include "../../../Module/Audio/AudioEffect.h"

ActionTrackToggleEffectEnabled::ActionTrackToggleEffectEnabled(AudioEffect *_fx)
{
	fx = _fx;
}

void *ActionTrackToggleEffectEnabled::execute(Data *d)
{
	fx->enabled = !fx->enabled;
	fx->Observable::notify(fx->MESSAGE_CHANGE_BY_ACTION);

	return nullptr;
}

void ActionTrackToggleEffectEnabled::undo(Data *d)
{
	execute(d);
}


