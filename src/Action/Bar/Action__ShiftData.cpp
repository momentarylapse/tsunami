/*
 * Action__ShiftData.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "Action__ShiftData.h"

#include "../../Data/Track.h"
#include <assert.h>

Action__ShiftData::Action__ShiftData(int _offset, int _shift)
{
	offset = _offset;
	shift = _shift;
}

void *Action__ShiftData::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	do_shift(s, shift);

	return NULL;
}

void Action__ShiftData::undo(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	offset += shift;
	do_shift(s, -shift);
	offset -= shift;
}

void Action__ShiftData::do_shift(Song *s, int delta)
{
	for (Track *t: s->tracks){

		// midi
		for (MidiNote *n: t->midi){
			if (n->range.start() >= offset)
				n->range.offset += delta;
		}

		// marker
		for (TrackMarker *m: t->markers)
			if (m->range.offset >= offset)
				m->range.offset += delta;

		// buffer
		for (TrackLayer &l: t->layers)
			for (AudioBuffer &b : l.buffers)
				if (b.offset >= offset)
					b.offset += delta;

		// marker
		for (SampleRef *s: t->samples)
			if (s->pos >= offset)
				s->pos += delta;
	}
}
