/*
 * ActionAudioSampleEditName.cpp
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#include "ActionAudioSampleEditName.h"
#include "../../../Data/AudioFile.h"

ActionAudioSampleEditName::ActionAudioSampleEditName(AudioFile *a, int _index, const string &name)
{
	index = _index;
	new_value = name;
	old_value = a->samples[index]->name;
}

ActionAudioSampleEditName::~ActionAudioSampleEditName()
{
}

void *ActionAudioSampleEditName::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Sample *s = a->samples[index];

	s->name = new_value;
	s->notify(s->MESSAGE_CHANGE_BY_ACTION);

	return NULL;
}

void ActionAudioSampleEditName::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Sample *s = a->samples[index];

	s->name = old_value;
}


bool ActionAudioSampleEditName::mergable(Action *a)
{
	ActionAudioSampleEditName *aa = dynamic_cast<ActionAudioSampleEditName*>(a);
	if (!aa)
		return false;
	return (aa->index == index);
}


