/*
 * ActionSongRenameLevel.cpp
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#include "../../Song/Level/ActionSongRenameLevel.h"

#include <assert.h>
#include "../../../Data/Song.h"

ActionSongRenameLevel::ActionSongRenameLevel(int _index, const string &_name)
{
	index = _index;
	name = _name;
}

ActionSongRenameLevel::~ActionSongRenameLevel()
{
}

void* ActionSongRenameLevel::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < a->level_names.num);

	string temp = name;
	name = a->level_names[index];
	a->level_names[index] = temp;

	a->notify(a->MESSAGE_EDIT_LEVEL);

	return NULL;
}

void ActionSongRenameLevel::undo(Data* d)
{
	execute(d);
}

