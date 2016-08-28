/*
 * ActionSong__DeleteLevel.cpp
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#include "ActionSong__DeleteLevel.h"
#include "../../../Data/Song.h"
#include <assert.h>

ActionSong__DeleteLevel::ActionSong__DeleteLevel(int _index)
{
	index = _index;
}

void* ActionSong__DeleteLevel::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < a->level_names.num);

	name = a->level_names[index];
	a->level_names.erase(index);

	for (Track *t : a->tracks){
		assert(t->levels[index].buffers.num == 0);
		t->levels.erase(index);
	}

	a->notify(a->MESSAGE_DELETE_LEVEL);

	return NULL;
}

void ActionSong__DeleteLevel::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);

	a->level_names.insert(name, index);

	TrackLevel new_level;
	for (Track *t : a->tracks)
		t->levels.insert(new_level, index);

	a->notify(a->MESSAGE_ADD_LEVEL);
}


