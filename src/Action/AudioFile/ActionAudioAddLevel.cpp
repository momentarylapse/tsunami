/*
 * ActionAudioAddLevel.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "ActionAudioAddLevel.h"
#include "../../Data/AudioFile.h"

ActionAudioAddLevel::ActionAudioAddLevel()
{
}

ActionAudioAddLevel::~ActionAudioAddLevel()
{
}

void* ActionAudioAddLevel::execute(Data* d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	a->level_name.add(format("level %d", a->level_name.num + 1));

	TrackLevel new_level;
	foreach(Track *t, a->track)
		t->level.add(new_level);

	a->cur_level = a->level_name.num - 1;

	return NULL;
}

void ActionAudioAddLevel::undo(Data* d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	a->level_name.pop();

	foreach(Track *t, a->track)
		t->level.pop();

	if (a->cur_level >= a->level_name.num)
		a->cur_level = a->level_name.num - 1;
}


