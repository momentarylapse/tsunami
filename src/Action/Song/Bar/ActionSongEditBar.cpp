/*
 * ActionSongEditBar.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionSongEditBar.h"
#include "ActionSong__EditBar.h"
#include "ActionSong__ScaleData.h"

#include "../../../Data/Track.h"
#include <assert.h>

ActionSongEditBar::ActionSongEditBar(int _index, BarPattern &_bar, bool _affect_data) :
	bar(_bar)
{
	index = _index;
	affect_data = _affect_data;
}

void ActionSongEditBar::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	Range r = Range(s->barOffset(index), s->bars[index].length);
	addSubAction(new ActionSong__EditBar(index, bar), d);
	if (affect_data)
		addSubAction(new ActionSong__ScaleData(r, bar.length), d);
}

