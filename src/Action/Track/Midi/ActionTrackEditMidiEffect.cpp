/*
 * ActionTrackEditMidiEffect.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ActionTrackEditMidiEffect.h"
#include "../../../Data/Track.h"
#include "../../../Plugins/MidiEffect.h"
#include <assert.h>

ActionTrackEditMidiEffect::ActionTrackEditMidiEffect(Track *t, int _index, const string &_old_params, MidiEffect *fx)
{
	track_no = get_track_index(t);
	index = _index;
	old_value = _old_params;
	new_value = fx->configToString();
}

ActionTrackEditMidiEffect::~ActionTrackEditMidiEffect()
{
}

void *ActionTrackEditMidiEffect::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	MidiEffect *fx = a->get_midi_fx(track_no, index);

	fx->configFromString(new_value);
	fx->Observable::notify(fx->MESSAGE_CHANGE_BY_ACTION);

	return NULL;
}

void ActionTrackEditMidiEffect::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	MidiEffect *fx = a->get_midi_fx(track_no, index);

	fx->configFromString(old_value);
	fx->Observable::notify(fx->MESSAGE_CHANGE_BY_ACTION);
}

bool ActionTrackEditMidiEffect::mergable(Action *a)
{
	ActionTrackEditMidiEffect *aa = dynamic_cast<ActionTrackEditMidiEffect*>(a);
	if (!aa)
		return false;
	return ((aa->track_no == track_no) && (aa->index == index));
}

