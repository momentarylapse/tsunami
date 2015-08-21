/*
 * ActionSongEditTag.cpp
 *
 *  Created on: 12.12.2012
 *      Author: michi
 */

#include "../../Song/Tag/ActionSongEditTag.h"

#include <assert.h>

ActionSongEditTag::ActionSongEditTag(int _index, const Tag &_tag)
{
	index = _index;
	new_tag = _tag;
}

ActionSongEditTag::~ActionSongEditTag()
{
}

void *ActionSongEditTag::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < a->tags.num);

	old_tag = a->tags[index];
	a->tags[index] = new_tag;

	return NULL;
}

void ActionSongEditTag::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->tags[index] = old_tag;
}

