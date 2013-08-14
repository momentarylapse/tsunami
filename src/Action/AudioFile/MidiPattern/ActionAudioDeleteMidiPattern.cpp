/*
 * ActionAudioDeleteMidiPattern.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionAudioDeleteMidiPattern.h"
#include "../../../Data/AudioFile.h"
#include <assert.h>

ActionAudioDeleteMidiPattern::ActionAudioDeleteMidiPattern(int _index)
{
	index = _index;
	pattern = NULL;
}

ActionAudioDeleteMidiPattern::~ActionAudioDeleteMidiPattern()
{
	if (pattern)
		if (!pattern->owner)
			delete(pattern);
}

void *ActionAudioDeleteMidiPattern::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(index >= 0);
	assert(index < a->midi_pattern.num);
	pattern = a->midi_pattern[index];
	assert(pattern->ref_count == 0);
	a->midi_pattern.erase(index);
	pattern->owner = NULL;
	return NULL;
}

void ActionAudioDeleteMidiPattern::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	pattern->owner = a;
	a->midi_pattern.insert(pattern, index);
}

