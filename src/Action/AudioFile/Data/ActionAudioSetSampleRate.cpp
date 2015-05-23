/*
 * ActionAudioSetSampleRate.cpp
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#include "ActionAudioSetSampleRate.h"
#include "../../../Data/AudioFile.h"

ActionAudioSetSampleRate::ActionAudioSetSampleRate(AudioFile *a, int _sample_rate)
{
	new_value = _sample_rate;
	old_value = a->sample_rate;
}

void *ActionAudioSetSampleRate::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	a->sample_rate = new_value;

	return NULL;
}

void ActionAudioSetSampleRate::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);

	a->sample_rate = old_value;
}


bool ActionAudioSetSampleRate::mergable(Action *a)
{
	ActionAudioSetSampleRate *aa = dynamic_cast<ActionAudioSetSampleRate*>(a);
	return aa;
}
