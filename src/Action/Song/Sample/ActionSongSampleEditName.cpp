/*
 * ActionSongSampleEditName.cpp
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#include "../../Song/Sample/ActionSongSampleEditName.h"
#include "../../../Data/Song.h"

ActionSongSampleEditName::ActionSongSampleEditName(Song *a, int _index, const string &name)
{
	index = _index;
	new_value = name;
	old_value = a->samples[index]->name;
}

ActionSongSampleEditName::~ActionSongSampleEditName()
{
}

void *ActionSongSampleEditName::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Sample *s = a->samples[index];

	s->name = new_value;
	s->notify(s->MESSAGE_CHANGE_BY_ACTION);

	return NULL;
}

void ActionSongSampleEditName::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Sample *s = a->samples[index];

	s->name = old_value;
}


bool ActionSongSampleEditName::mergable(Action *a)
{
	ActionSongSampleEditName *aa = dynamic_cast<ActionSongSampleEditName*>(a);
	if (!aa)
		return false;
	return (aa->index == index);
}


