/*
 * ActionTrackAddEffect.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackAddEffect.h"
#include "../../../Data/Track.h"

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
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	if (track_no >= 0){
		Track *t = a->get_track(track_no);
		t->fx.add(effect);
		t->Notify("AddEffect");
	}else{
		a->fx.add(effect);
	}

	return NULL;
}

void ActionTrackAddEffect::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	effect->Notify("Delete");
	if (track_no >= 0){
		Track *t = a->get_track(track_no);
		t->fx.pop();
		t->Notify("DeleteEffect");
	}else{
		a->fx.pop();
	}
}

