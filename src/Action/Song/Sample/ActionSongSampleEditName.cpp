/*
 * ActionSongSampleEditName.cpp
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#include "../../Song/Sample/ActionSongSampleEditName.h"
#include "../../../Data/Song.h"

ActionSongSampleEditName::ActionSongSampleEditName(Sample *s, const string &name)
{
	sample = s;
	new_value = name;
	old_value = s->name;
}

ActionSongSampleEditName::~ActionSongSampleEditName()
{
}

void *ActionSongSampleEditName::execute(Data *d)
{
	sample->name = new_value;
	sample->notify(sample->MESSAGE_CHANGE_BY_ACTION);

	return NULL;
}

void ActionSongSampleEditName::undo(Data *d)
{
	sample->name = old_value;
	sample->notify(sample->MESSAGE_CHANGE_BY_ACTION);
}


bool ActionSongSampleEditName::mergable(Action *a)
{
	ActionSongSampleEditName *aa = dynamic_cast<ActionSongSampleEditName*>(a);
	if (!aa)
		return false;
	return (aa->sample == sample);
}


