/*
 * ActionTrackAddEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackAddEffect.h"
#include "../../../Data/Track.h"
#include "../../../Plugins/Effect.h"

ActionTrackAddEffect::ActionTrackAddEffect(Track *t, Effect *_effect)
{
	track_no = get_track_index(t);
	effect = _effect;
}

ActionTrackAddEffect::~ActionTrackAddEffect()
{
}

void *ActionTrackAddEffect::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	if (track_no >= 0){
		Track *t = a->get_track(track_no);
		t->fx.add(effect);
		t->notify(t->MESSAGE_ADD_EFFECT);
	}else{
		a->fx.add(effect);
		a->notify(a->MESSAGE_ADD_EFFECT);
	}

	return NULL;
}

void ActionTrackAddEffect::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	effect->Observable::notify(effect->MESSAGE_DELETE);
	if (track_no >= 0){
		Track *t = a->get_track(track_no);
		t->fx.pop();
		t->notify(t->MESSAGE_DELETE_EFFECT);
	}else{
		a->fx.pop();
		a->notify(a->MESSAGE_DELETE_EFFECT);
	}
}

