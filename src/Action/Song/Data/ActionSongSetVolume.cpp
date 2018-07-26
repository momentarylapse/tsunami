/*
 * ActionSongSetVolume.cpp
 *
 *  Created on: 20.09.2013
 *      Author: michi
 */

#include "ActionSongSetVolume.h"
#include "../../../Data/Song.h"

ActionSongSetVolume::ActionSongSetVolume(Song *s, float _volume)
{
	new_value = _volume;
	old_value = s->volume;
}

void *ActionSongSetVolume::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	s->volume = new_value;

	return nullptr;
}

void ActionSongSetVolume::undo(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	s->volume = old_value;
}


bool ActionSongSetVolume::mergable(Action *a)
{
	ActionSongSetVolume *aa = dynamic_cast<ActionSongSetVolume*>(a);
	return aa;
}

