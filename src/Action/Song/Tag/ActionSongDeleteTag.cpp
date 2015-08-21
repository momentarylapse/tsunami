/*
 * ActionSongDeleteTag.cpp
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#include <assert.h>
#include "ActionSongDeleteTag.h"

ActionSongDeleteTag::ActionSongDeleteTag(int _index)
{
	index = _index;
}

ActionSongDeleteTag::~ActionSongDeleteTag()
{
}

void *ActionSongDeleteTag::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < a->tags.num);

	old_tag = a->tags[index];
	a->tags.erase(index);

	return NULL;
}

void ActionSongDeleteTag::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->tags.insert(old_tag, index);
}

