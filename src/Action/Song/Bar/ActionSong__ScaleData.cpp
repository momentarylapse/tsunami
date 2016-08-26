/*
 * ActionSong__ScaleData.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "ActionSong__ScaleData.h"

#include "../../../Data/Track.h"
#include <assert.h>

ActionSong__ScaleData::ActionSong__ScaleData(const Range &_source, int _new_size)
{
	source = _source;
	new_size = _new_size;
}

void *ActionSong__ScaleData::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	do_scale(s, source, new_size);

	return NULL;
}

void ActionSong__ScaleData::undo(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	do_scale(s, Range(source.offset, new_size), source.length);
}

int __shift_data_shift(const Range &source, int new_length, int pos)
{
	if (pos >= source.end())
		// after source
		return pos + new_length - source.length;
	if (pos >= source.offset){
		// inside source
		if (source.length == 0)
			return pos - new_length;
		return source.offset + (float)(pos - source.offset) * (float)new_length / (float)source.length;
	}
	return pos;
}

void ActionSong__ScaleData::do_scale(Song *s, const Range &r, int new_length)
{
	int pos0 = r.offset;
	for (Track *t : s->tracks){

		// midi
		foreachi(MidiNote &n, t->midi, j){
			// note start
			if (n.range.start() >= pos0)
				n.range.set_start(__shift_data_shift(r, new_length, n.range.start()));
			// note end
			if (n.range.end() >= pos0)
				n.range.set_end(__shift_data_shift(r, new_length, n.range.end()));
		}

		// marker
		for (TrackMarker &m : t->markers)
			if (m.pos >= pos0)
				m.pos = __shift_data_shift(r, new_length, m.pos);

		// buffer
		for (TrackLevel &l : t->levels)
			for (BufferBox &b : l.buffers)
				if (b.offset >= pos0)
					b.offset = __shift_data_shift(r, new_length, b.offset);

		// marker
		for (SampleRef *s : t->samples)
			if (s->pos >= pos0)
				s->pos = __shift_data_shift(r, new_length, s->pos);
	}
}
