/*
 * ActionTrackToggleEffectEnabled.cpp
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#include "ActionTrackToggleEffectEnabled.h"
#include "../../../Data/Track.h"
#include "../../../Plugins/Effect.h"

ActionTrackToggleEffectEnabled::ActionTrackToggleEffectEnabled(Track *t, int _index)
{
	track_no = get_track_index(t);
	index = _index;
}

void *ActionTrackToggleEffectEnabled::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	Effect *fx = a->get_fx(track_no, index);
	fx->enabled = !fx->enabled;
	fx->Observable::notify(fx->MESSAGE_CHANGE_BY_ACTION);

	return NULL;
}

void ActionTrackToggleEffectEnabled::undo(Data *d)
{
	execute(d);
}


