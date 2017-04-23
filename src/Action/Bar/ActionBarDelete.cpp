/*
 * ActionSongBarDelete.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionBarDelete.h"

#include "../Song/ActionSongDeleteSelection.h"

#include "../../Data/Song.h"
#include "../../Data/SongSelection.h"
#include <assert.h>

#include "Action__ShiftData.h"
#include "ActionBar__Delete.h"

ActionBarDelete::ActionBarDelete(int _index, bool _affect_data)
{
	index = _index;
	affect_data = _affect_data;
}

void ActionBarDelete::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0 and index < s->bars.num);

	Range r = Range(s->barOffset(index), s->bars[index].length);
	addSubAction(new ActionBar__Delete(index), d);

	if (affect_data){
		SongSelection sel;
		sel.all_tracks(s);
		sel.fromRange(s, r);
		addSubAction(new ActionSongDeleteSelection(-1, sel, true), d);

		addSubAction(new Action__ShiftData(r.end(), - r.length), d);

	}
}
