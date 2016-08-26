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

ActionSongEditBar::ActionSongEditBar(Song *s, int index, BarPattern &bar, bool affect_data)
{
	Range r = Range(s->barOffset(index), s->bars[index].length);
	addSubAction(new ActionSong__EditBar(index, bar), s);
	if (affect_data){
		addSubAction(new ActionSong__ScaleData(r, bar.length), s);

	}
}

