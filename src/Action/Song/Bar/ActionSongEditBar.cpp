/*
 * ActionSongEditBar.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionSongEditBar.h"

#include "../../../Data/Track.h"
#include <assert.h>

ActionSongEditBar::ActionSongEditBar(int _index, BarPattern &_bar, bool _affect_midi)
{
	index = _index;
	bar = _bar;
	affect_midi = _affect_midi;
}

void *ActionSongEditBar::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < s->bars.num);

	if (affect_midi){
		int pos = s->barOffset(index);
		int l0 = s->bars[index].length;
		s->__shift_data(Range(pos, l0), bar.length);
	}

	BarPattern temp = bar;
	bar = s->bars[index];
	s->bars[index] = temp;
	s->notify(s->MESSAGE_EDIT_BARS);

	return NULL;
}

void ActionSongEditBar::undo(Data *d)
{
	execute(d);
}

