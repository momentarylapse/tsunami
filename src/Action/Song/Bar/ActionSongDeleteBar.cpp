/*
 * ActionSongDeleteBar.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionSongDeleteBar.h"
#include "ActionSong__DeleteBar.h"
#include "ActionSong__ShiftData.h"
#include "../ActionSongDeleteSelection.h"

#include "../../../Data/Song.h"
#include "../../../Data/SongSelection.h"
#include <assert.h>

ActionSongDeleteBar::ActionSongDeleteBar(int _index, bool _affect_data)
{
	index = _index;
	affect_data = _affect_data;
}

void ActionSongDeleteBar::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0 and index < s->bars.num);

	Range r = Range(s->barOffset(index), s->bars[index].length);
	addSubAction(new ActionSong__DeleteBar(index), d);

	if (affect_data){
		SongSelection sel;
		sel.all_tracks(s);
		sel.fromRange(s, r);
		addSubAction(new ActionSongDeleteSelection(-1, sel, true), d);

		addSubAction(new ActionSong__ShiftData(r.end(), - r.length), d);

	}
}
