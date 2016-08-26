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

ActionSongDeleteBar::ActionSongDeleteBar(Song *s, int index, bool affect_midi)
{
	assert(index >= 0 and index < s->bars.num);

	Range r = Range(s->barOffset(index), s->bars[index].length);
	addSubAction(new ActionSong__DeleteBar(index), s);

	if (affect_midi){
		SongSelection sel;
		sel.all_tracks(s);
		sel.fromRange(s, r);
		addSubAction(new ActionSongDeleteSelection(s, -1, sel, true), s);

		addSubAction(new ActionSong__ShiftData(r.end(), - r.length), s);

	}
}
