/*
 * ActionTrackToggleMidiEffectEnabled.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ActionTrackToggleMidiEffectEnabled.h"
#include "../../../Data/Track.h"
#include "../../../Plugins/MidiEffect.h"

ActionTrackToggleMidiEffectEnabled::ActionTrackToggleMidiEffectEnabled(Track *t, int _index)
{
	track_no = get_track_index(t);
	index = _index;
}

ActionTrackToggleMidiEffectEnabled::~ActionTrackToggleMidiEffectEnabled()
{
}

void *ActionTrackToggleMidiEffectEnabled::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	MidiEffect *fx = a->get_midi_fx(track_no, index);
	fx->enabled = !fx->enabled;
	fx->Observable::notify(fx->MESSAGE_CHANGE_BY_ACTION);

	return NULL;
}

void ActionTrackToggleMidiEffectEnabled::undo(Data *d)
{
	execute(d);
}


