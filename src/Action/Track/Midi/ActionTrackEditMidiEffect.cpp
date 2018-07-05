/*
 * ActionTrackEditMidiEffect.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ActionTrackEditMidiEffect.h"
#include "../../../Data/Track.h"
#include <assert.h>

#include "../../../Module/Midi/MidiEffect.h"

ActionTrackEditMidiEffect::ActionTrackEditMidiEffect(MidiEffect *_fx, const string &_old_params)
{
	fx = _fx;
	old_value = _old_params;
	new_value = fx->config_to_string();
}

void *ActionTrackEditMidiEffect::execute(Data *d)
{
	fx->config_from_string(new_value);
	fx->Observable::notify(fx->MESSAGE_CHANGE_BY_ACTION);

	return NULL;
}

void ActionTrackEditMidiEffect::undo(Data *d)
{
	fx->config_from_string(old_value);
	fx->Observable::notify(fx->MESSAGE_CHANGE_BY_ACTION);
}

bool ActionTrackEditMidiEffect::mergable(Action *a)
{
	ActionTrackEditMidiEffect *aa = dynamic_cast<ActionTrackEditMidiEffect*>(a);
	if (!aa)
		return false;
	return (aa->fx == fx);
}

