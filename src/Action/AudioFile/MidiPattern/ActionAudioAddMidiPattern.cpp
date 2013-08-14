/*
 * ActionAudioAddMidiPattern.cpp
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#include "ActionAudioAddMidiPattern.h"
#include "../../../Data/AudioFile.h"
#include <assert.h>

ActionAudioAddMidiPattern::ActionAudioAddMidiPattern(const string &name, int num_beats, int beat_partition)
{
	pattern = new MidiPattern;
	pattern->num_beats = num_beats;
	pattern->beat_partition = beat_partition;
	pattern->name = name;
}

ActionAudioAddMidiPattern::~ActionAudioAddMidiPattern()
{
	if (!pattern->owner)
		delete(pattern);
}

void *ActionAudioAddMidiPattern::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	pattern->owner = a;
	a->midi_pattern.add(pattern);
	return pattern;
}

void ActionAudioAddMidiPattern::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	assert(pattern->ref_count == 0);
	a->midi_pattern.pop();
	pattern->owner = NULL;
}

