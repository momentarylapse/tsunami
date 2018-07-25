/*
 * Action__ShiftData.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "Action__ShiftData.h"

#include "../../Data/Track.h"
#include "../../Data/Song.h"
#include "../../Data/SampleRef.h"
#include "../../Data/Audio/AudioBuffer.h"
#include <assert.h>

Action__ShiftData::Action__ShiftData(int _offset, int _shift, int _mode)
{
	offset = _offset;
	shift = _shift;
	mode = _mode;
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


		// marker
		for (TrackMarker *m: t->markers){
			if (m->range.offset >= offset)
				m->range.offset += delta;
			/*else if (m->range.end() >= offset)
				m->range.length += delta;*/
		}

		for (TrackLayer *l: t->layers){
			// buffer
			for (AudioBuffer &b : l->buffers)
				if (b.offset >= offset)
					b.offset += delta;

			// midi
			for (MidiNote *n: l->midi){
				if (n->range.start() >= offset)
					n->range.offset += delta;
				/*else if (n->range.end() >= offset){
					printf("end... %p %d\n", n, n->range.end());
					n->range.length += delta;
				}*/
			}

			// samples
			for (SampleRef *s: l->samples)
				if (s->pos >= offset)
					s->pos += delta;
		}
	}
}
