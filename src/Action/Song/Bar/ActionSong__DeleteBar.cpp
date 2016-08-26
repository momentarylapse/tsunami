/*
 * ActionSong__DeleteBar.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "ActionSong__DeleteBar.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionSong__DeleteBar::ActionSong__DeleteBar(int _index)
{
	index = _index;
}

void *ActionSong__DeleteBar::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < s->bars.num);

	bar = s->bars[index];
	s->bars.erase(index);
	s->notify(s->MESSAGE_EDIT_BARS);

	return NULL;
}

void ActionSong__DeleteBar::undo(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index <= s->bars.num);

	s->bars.insert(bar, index);
	s->notify(s->MESSAGE_EDIT_BARS);
}
