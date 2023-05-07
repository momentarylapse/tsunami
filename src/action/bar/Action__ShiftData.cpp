/*
 * Action__ShiftData.cpp
 *
 *  Created on: 26.08.2016
 *      Author: michi
 */

#include "Action__ShiftData.h"

#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/TrackMarker.h"
#include "../../data/CrossFade.h"
#include "../../data/Song.h"
#include "../../data/SampleRef.h"
#include "../../data/audio/AudioBuffer.h"
#include <assert.h>

Action__ShiftData::Action__ShiftData(int _offset, int _shift, int _mode) {
	offset = _offset;
	shift = _shift;
	mode = _mode;
}

void *Action__ShiftData::execute(Data *d) {
	Song *s = dynamic_cast<Song*>(d);

	do_shift(s, shift);

	return nullptr;
}

void Action__ShiftData::undo(Data *d) {
	Song *s = dynamic_cast<Song*>(d);

	offset += shift;
	do_shift(s, -shift);
	offset -= shift;
}

void Action__ShiftData::do_shift(Song *s, int delta) {
	for (auto t: weak(s->tracks)) {

		for (auto l: weak(t->layers)) {
			// buffer
			for (auto &b : l->buffers)
				if (b.offset >= offset)
					b.offset += delta;

			// midi
			for (auto n: weak(l->midi)) {
				if (n->range.start() >= offset)
					n->range.offset += delta;
				/*else if (n->range.end() >= offset){
					printf("end... %p %d\n", n, n->range.end());
					n->range.length += delta;
				}*/
			}

			// marker
			for (auto m: weak(l->markers)) {
				if (m->range.offset >= offset)
					m->range.offset += delta;
				/*else if (m->range.end() >= offset)
					m->range.length += delta;*/
			}

			// samples
			for (auto s: weak(l->samples))
				if (s->pos >= offset)
					s->pos += delta;

			for (auto &f: l->fades)
				if (f.position >= offset)
					f.position += delta;

			l->out_changed.notify();
		}
	}
}
