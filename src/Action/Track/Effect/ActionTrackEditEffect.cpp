/*
 * ActionTrackEditEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditEffect.h"
#include "../../../Data/Track.h"
#include "../../../Plugins/Effect.h"
#include <assert.h>

ActionTrackEditEffect::ActionTrackEditEffect(Track *t, int _index, const string &_old_params, Effect *fx)
{
	track_no = get_track_index(t);
	index = _index;
	old_value = _old_params;
	new_value = fx->configToString();
}

void *ActionTrackEditEffect::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	Effect *fx = a->get_fx(track_no, index);

	fx->configFromString(new_value);
	fx->Observable::notify(fx->MESSAGE_CHANGE_BY_ACTION);

	return NULL;
}

void ActionTrackEditEffect::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	Effect *fx = a->get_fx(track_no, index);

	fx->configFromString(old_value);
	fx->Observable::notify(fx->MESSAGE_CHANGE_BY_ACTION);
}

bool ActionTrackEditEffect::mergable(Action *a)
{
	ActionTrackEditEffect *aa = dynamic_cast<ActionTrackEditEffect*>(a);
	if (!aa)
		return false;
	return ((aa->track_no == track_no) && (aa->index == index));
}

