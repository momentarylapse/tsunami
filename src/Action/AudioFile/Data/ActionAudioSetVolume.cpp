/*
 * ActionAudioSetVolume.cpp
 *
 *  Created on: 20.09.2013
 *      Author: michi
 */

#include "ActionAudioSetVolume.h"
#include "../../../Data/AudioFile.h"

ActionAudioSetVolume::ActionAudioSetVolume(AudioFile *a, float _volume)
{
	new_value = _volume;
	old_value = a->volume;
}

ActionAudioSetVolume::~ActionAudioSetVolume()
{
}

void *ActionAudioSetVolume::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	a->volume = new_value;

	return NULL;
}

void ActionAudioSetVolume::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	a->volume = old_value;
}


bool ActionAudioSetVolume::mergable(Action *a)
{
	ActionAudioSetVolume *aa = dynamic_cast<ActionAudioSetVolume*>(a);
	return aa;
}

