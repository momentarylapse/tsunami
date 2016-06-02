/*
 * ActionSongAddLevel.cpp
 *
 *  Created on: 11.07.2012
 *      Author: michi
 */

#include "../../Song/Level/ActionSongAddLevel.h"
#include "../../../Data/Song.h"

ActionSongAddLevel::ActionSongAddLevel(const string &_name)
{
	name = _name;
}

ActionSongAddLevel::~ActionSongAddLevel()
{
}

void* ActionSongAddLevel::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	a->level_names.add(name);

	TrackLevel new_level;
	for (Track *t : a->tracks)
		t->levels.add(new_level);
	a->notify(a->MESSAGE_ADD_LEVEL);

	return NULL;
}

void ActionSongAddLevel::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->level_names.pop();

	for (Track *t : a->tracks)
		t->levels.pop();

	a->notify(a->MESSAGE_DELETE_LEVEL);
}


