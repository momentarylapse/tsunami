/*
 * ActionSong__ShiftData.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "ActionSong__ShiftData.h"

#include "../../../Data/Track.h"
#include <assert.h>

ActionSong__ShiftData::ActionSong__ShiftData(int _offset, int _shift)
{
	offset = _offset;
	shift = _shift;
}

void *ActionSong__ShiftData::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	do_shift(s, shift);

	return NULL;
}

void ActionSong__ShiftData::undo(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	do_shift(s, -shift);
}

void ActionSong__ShiftData::do_shift(Song *s, int delta)
{
	for (Track *t: s->tracks){

		// midi
		for (MidiNote *n: t->midi){
			if (n->range.start() >= offset)
				n->range.offset += delta;
		}

		// marker
		for (TrackMarker *m: t->markers)
			if (m->pos >= offset)
				m->pos += delta;

		// buffer
		for (TrackLevel &l: t->levels)
			for (BufferBox &b : l.buffers)
				if (b.offset >= offset)
					b.offset += delta;

		// marker
		for (SampleRef *s: t->samples)
			if (s->pos >= offset)
				s->pos += delta;
	}
}
