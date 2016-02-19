/*
 * ActionSongAddBar.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionSongAddBar.h"

#include "../../../Data/Track.h"
#include <assert.h>

ActionSongAddBar::ActionSongAddBar(int _index, BarPattern &_bar, bool _affect_midi)
{
	index = _index;
	bar = _bar;
	affect_midi = _affect_midi;
}

void *ActionSongAddBar::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index <= s->bars.num);

	if (affect_midi){
		int pos0 = s->barOffset(index);
		s->__shift_data(Range(pos0, 0), bar.length);
	}

	s->bars.insert(bar, index);
	s->notify(s->MESSAGE_EDIT_BARS);

	return NULL;
}

void ActionSongAddBar::undo(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	if (affect_midi){
		int pos0 = s->barOffset(index);
		s->__shift_data(Range(pos0, bar.length), 0);
	}

	s->bars.erase(index);
	s->notify(s->MESSAGE_EDIT_BARS);
}

