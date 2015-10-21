/*
 * ActionSongDeleteBar.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionSongDeleteBar.h"

#include "../../../Data/Track.h"
#include <assert.h>

ActionSongDeleteBar::ActionSongDeleteBar(int _index, bool _affect_midi)
{
	index = _index;
	affect_midi = _affect_midi;
}

void *ActionSongDeleteBar::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < s->bars.num);

	bar = s->bars[index];
	s->bars.erase(index);
	s->notify(s->MESSAGE_EDIT_BARS);

	return NULL;
}

void ActionSongDeleteBar::undo(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index <= s->bars.num);

	s->bars.insert(bar, index);
	s->notify(s->MESSAGE_EDIT_BARS);
}

