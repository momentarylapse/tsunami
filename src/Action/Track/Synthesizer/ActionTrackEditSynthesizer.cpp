/*
 * ActionTrackEditSynthesizer.cpp
 *
 *  Created on: 29.12.2013
 *      Author: michi
 */

#include "ActionTrackEditSynthesizer.h"
#include "../../../Data/Track.h"
#include "../../../Module/Synth/Synthesizer.h"
#include <assert.h>

ActionTrackEditSynthesizer::ActionTrackEditSynthesizer(Track *t, const string &params_old)
{
	track = t;
	old_value = params_old;
	new_value = t->synth->config_to_string();
}

void *ActionTrackEditSynthesizer::execute(Data *d)
{
	track->synth->config_from_string(new_value);
	track->synth->Observable::notify(track->synth->MESSAGE_CHANGE_BY_ACTION);

	return nullptr;
}

void ActionTrackEditSynthesizer::undo(Data *d)
{
	track->synth->config_from_string(old_value);
	track->synth->Observable::notify(track->synth->MESSAGE_CHANGE_BY_ACTION);
}

bool ActionTrackEditSynthesizer::mergable(Action *a)
{
	ActionTrackEditSynthesizer *aa = dynamic_cast<ActionTrackEditSynthesizer*>(a);
	if (!aa)
		return false;
	return (aa->track == track);
}

