/*
 * ActionSongAddBar.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionSongAddBar.h"
#include "ActionSong__AddBar.h"
#include "ActionSong__ShiftData.h"
#include "../../Track/Buffer/ActionTrack__SplitBufferBox.h"

#include "../../../Data/Track.h"
#include <assert.h>

ActionSongAddBar::ActionSongAddBar(Song *s, int index, BarPattern &bar, bool affect_midi)
{
	addSubAction(new ActionSong__AddBar(index, bar), s);

	if (affect_midi){
		int pos0 = s->barOffset(index);

		for (Track *t : s->tracks)
			for (int l=0; l<t->levels.num; l++)
				for (int i=t->levels[l].buffers.num-1; i>=0; i--)
					if (t->levels[l].buffers[i].range().is_more_inside(pos0))
						addSubAction(new ActionTrack__SplitBufferBox(t, l, i, pos0 - t->levels[l].buffers[i].offset), s);

		addSubAction(new ActionSong__ShiftData(pos0, bar.length), s);

	}
}

