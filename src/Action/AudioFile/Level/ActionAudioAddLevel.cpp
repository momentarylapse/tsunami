/*
 * ActionAudioAddLevel.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionAudioAddLevel.h"
#include "../../../Data/AudioFile.h"

ActionAudioAddLevel::ActionAudioAddLevel(const string &_name)
{
	name = _name;
}

ActionAudioAddLevel::~ActionAudioAddLevel()
{
}

void* ActionAudioAddLevel::execute(Data* d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	a->level_names.add(name);

	TrackLevel new_level;
	foreach(Track *t, a->tracks)
		t->levels.add(new_level);

	return NULL;
}

void ActionAudioAddLevel::undo(Data* d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	a->level_names.pop();

	foreach(Track *t, a->tracks)
		t->levels.pop();
}


