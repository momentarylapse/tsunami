/*
 * ActionTrackAddMidiEffect.cpp
 *
 *  Created on: 23.09.2014
 *      Author: michi
 */

#include "ActionTrackAddMidiEffect.h"
#include "../../../Data/Track.h"
#include "../../../Plugins/MidiEffect.h"

ActionTrackAddMidiEffect::ActionTrackAddMidiEffect(Track *t, MidiEffect *_effect)
{
	track_no = get_track_index(t);
	effect = _effect;
}

ActionTrackAddMidiEffect::~ActionTrackAddMidiEffect()
{
}

void *ActionTrackAddMidiEffect::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	Track *t = a->get_track(track_no);
	t->midi.fx.add(effect);
	t->Notify(t->MESSAGE_ADD_EFFECT);

	return NULL;
}

void ActionTrackAddMidiEffect::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	effect->Notify(effect->MESSAGE_DELETE);
	Track *t = a->get_track(track_no);
	t->midi.fx.pop();
	t->Notify(t->MESSAGE_DELETE_EFFECT);
}

