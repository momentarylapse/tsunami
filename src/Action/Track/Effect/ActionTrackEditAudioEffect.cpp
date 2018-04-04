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

ActionTrackEditEffect::ActionTrackEditEffect(Track *t, int _index, const string &_old_params, AudioEffect *fx)
{
	track_no = get_track_index(t);
	index = _index;
	old_value = _old_params;
	new_value = fx->config_to_string();
}

void *ActionTrackEditEffect::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	AudioEffect *fx = a->get_fx(track_no, index);

	fx->config_from_string(new_value);
	fx->Observable::notify(fx->MESSAGE_CHANGE_BY_ACTION);

	return NULL;
}

void ActionTrackEditEffect::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	AudioEffect *fx = a->get_fx(track_no, index);

	fx->config_from_string(old_value);
	fx->Observable::notify(fx->MESSAGE_CHANGE_BY_ACTION);
}

bool ActionTrackEditEffect::mergable(Action *a)
{
	ActionTrackEditEffect *aa = dynamic_cast<ActionTrackEditEffect*>(a);
	if (!aa)
		return false;
	return ((aa->track_no == track_no) and (aa->index == index));
}

