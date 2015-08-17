/*
 * ActionSongAddTag.cpp
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#include "ActionSongAddTag.h"

ActionSongAddTag::ActionSongAddTag(const Tag &_tag)
{
	tag = _tag;
}

ActionSongAddTag::~ActionSongAddTag()
{
}

void *ActionSongAddTag::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->tags.add(tag);

	return NULL;
}

void ActionSongAddTag::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->tags.pop();
}

