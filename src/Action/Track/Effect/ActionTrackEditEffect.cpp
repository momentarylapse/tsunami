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
	new_value = fx->ConfigToString();
}

ActionTrackEditEffect::~ActionTrackEditEffect()
{
}

void *ActionTrackEditEffect::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	Effect *fx = a->get_fx(track_no, index);

	fx->ConfigFromString(new_value);
	fx->Notify("ChangeByAction");

	return NULL;
}

void ActionTrackEditEffect::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	Effect *fx = a->get_fx(track_no, index);

	fx->ConfigFromString(old_value);
	fx->Notify("ChangeByAction");
}

bool ActionTrackEditEffect::mergable(Action *a)
{
	ActionTrackEditEffect *aa = dynamic_cast<ActionTrackEditEffect*>(a);
	if (!aa)
		return false;
	return ((aa->track_no == track_no) && (aa->index == index));
}

