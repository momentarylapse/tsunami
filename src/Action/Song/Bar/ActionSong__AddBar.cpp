/*
 * ActionSong__AddBar.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "ActionSong__AddBar.h"

#include "../../../Data/Track.h"
#include <assert.h>

ActionSong__AddBar::ActionSong__AddBar(int _index, BarPattern &_bar)
{
	index = _index;
	bar = _bar;
}

void *ActionSong__AddBar::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index <= s->bars.num);

	s->bars.insert(bar, index);
	s->notify(s->MESSAGE_EDIT_BARS);

	return NULL;
}

void ActionSong__AddBar::undo(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	s->bars.erase(index);
	s->notify(s->MESSAGE_EDIT_BARS);
}

