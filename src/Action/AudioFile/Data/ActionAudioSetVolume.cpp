/*
 * ActionAudioSetVolume.cpp
 *
 *  Created on: 20.09.2013
 *      Author: michi
 */

#include "ActionAudioSetVolume.h"
#include "../../../Data/AudioFile.h"

ActionAudioSetVolume::ActionAudioSetVolume(float _volume)
{
	volume = _volume;
}

ActionAudioSetVolume::~ActionAudioSetVolume()
{
}

void *ActionAudioSetVolume::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	float t = volume;
	volume = a->volume;
	a->volume = t;

	return NULL;
}

void ActionAudioSetVolume::undo(Data *d)
{
	execute(d);
}

