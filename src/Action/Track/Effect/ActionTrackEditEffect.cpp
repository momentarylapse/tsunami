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

Effect *ActionTrackEditEffect::get_fx(AudioFile *a)
{
	if (track_no >= 0){
		Track *t = a->get_track(track_no);
		assert(t);
		assert(index < t->fx.num);

		return t->fx[index];
	}else{
		assert(index < a->fx.num);

		return a->fx[index];
	}
}

void *ActionTrackEditEffect::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	Effect *fx = get_fx(a);

	fx->ConfigFromString(new_value);
	fx->Notify("ChangeByAction");

	return NULL;
}

void ActionTrackEditEffect::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	Effect *fx = get_fx(a);

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

