/*
 * ActionSongDeleteLevel.cpp
 *
 *  Created on: 27.08.2016
 *      Author: michi
 */

#include "ActionSongDeleteLevel.h"
#include "ActionSong__DeleteLevel.h"
#include "../../Track/Buffer/ActionTrack__DeleteBufferBox.h"
#include "../../../Data/Song.h"

ActionSongDeleteLevel::ActionSongDeleteLevel(int _index)
{
	index = _index;
}

void ActionSongDeleteLevel::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	for (Track* t : s->tracks){
		TrackLevel &l = t->levels[index];
		for (int i=l.buffers.num-1; i>=0; i--)
			addSubAction(new ActionTrack__DeleteBufferBox(t, index, i), s);
	}

	addSubAction(new ActionSong__DeleteLevel(index), s);
}

