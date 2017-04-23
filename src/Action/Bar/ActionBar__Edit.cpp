/*
 * ActionBar__Edit.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "ActionBar__Edit.h"

#include "../../Data/Track.h"
#include <assert.h>


ActionBar__Edit::ActionBar__Edit(int _index, BarPattern &_bar)
{
	index = _index;
	bar = _bar;
}

void *ActionBar__Edit::execute(Data *d)
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

void ActionBar__Edit::undo(Data *d)
{
	execute(d);
}

