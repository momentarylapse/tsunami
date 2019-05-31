/*
 * Action__ScaleData.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "Action__ScaleData.h"

#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/TrackMarker.h"
#include "../../Data/CrossFade.h"
#include "../../Data/Song.h"
#include "../../Data/SampleRef.h"
#include "../../Data/Audio/AudioBuffer.h"
#include <assert.h>

Action__ScaleData::Action__ScaleData(const Range &_source, int _new_size)
{
	source = _source;
	new_size = _new_size;
}

void *Action__ScaleData::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	do_scale(s, source, new_size);

	return nullptr;
}

void Action__ScaleData::undo(Data *d)
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
		return source.offset + (int)((float)(pos - source.offset) * (float)new_length / (float)source.length);
	}
	return pos;
}

void Action__ScaleData::do_scale(Song *s, const Range &r, int new_length)
{
	int pos0 = r.offset;
	for (Track *t: s->tracks){

		// marker
		for (TrackMarker *m: t->markers){
			if (m->range.start() >= pos0)
				m->range.set_start(__shift_data_shift(r, new_length, m->range.start()));
			if (m->range.end() >= pos0)
				m->range.set_end(__shift_data_shift(r, new_length, m->range.end()));
		}

		// fades
		for (CrossFade &f: t->fades)
			if (f.position >= pos0)
				f.position = __shift_data_shift(r, new_length, f.position);

		// buffer
		for (TrackLayer *l: t->layers){
			for (AudioBuffer &b: l->buffers)
				if (b.offset >= pos0)
					b.offset = __shift_data_shift(r, new_length, b.offset);


			// midi
			foreachi(MidiNote *n, l->midi, j){
				// note start
				if (n->range.start() >= pos0)
					n->range.set_start(__shift_data_shift(r, new_length, n->range.start()));
				// note end
				if (n->range.end() >= pos0)
					n->range.set_end(__shift_data_shift(r, new_length, n->range.end()));
			}

			// samples
			for (SampleRef *s: l->samples)
				if (s->pos >= pos0)
					s->pos = __shift_data_shift(r, new_length, s->pos);
		}
	}
}
