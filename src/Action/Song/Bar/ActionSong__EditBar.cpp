/*
 * ActionSong__EditBar.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "ActionSong__EditBar.h"

#include "../../../Data/Track.h"
#include <assert.h>

ActionSong__EditBar::ActionSong__EditBar(int _index, BarPattern &_bar)
{
	index = _index;
	bar = _bar;
}

void *ActionSong__EditBar::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < s->bars.num);

	BarPattern temp = bar;
	bar = s->bars[index];
	s->bars[index] = temp;
	s->notify(s->MESSAGE_EDIT_BARS);

	return NULL;
}

void ActionSong__EditBar::undo(Data *d)
{
	execute(d);
}

