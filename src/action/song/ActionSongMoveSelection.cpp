/*
 * ActionSongMoveSelection.cpp
 *
 *  Created on: 17.07.2018
 *      Author: michi
 */

#include "ActionSongMoveSelection.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/TrackMarker.h"
#include "../../data/SongSelection.h"
#include "../../data/SampleRef.h"
#include "../../data/audio/AudioBuffer.h"


ActionSongMoveSelection::ActionSongMoveSelection(Song *a, const SongSelection &sel, bool move_buffers) {
	for (auto t: weak(a->tracks)) {
		if (sel.has(t))
			tracks.add(t);

		for (auto l: weak(t->layers)) {
			for (auto n: weak(l->midi))
				if (sel.has(n))
					notes.add({n, n->range.offset});
			for (auto m: weak(l->markers))
				if (sel.has(m))
					markers.add({m, m->range.offset});
			for (auto s: weak(l->samples))
				if (sel.has(s))
					samples.add({s, s->pos});
			if (move_buffers and sel.has(l)) {
				foreachi(AudioBuffer &b, l->buffers, i) {
					if (b.range().overlaps(sel.range()))
						buffers.add({l, i, b.offset});
				}
			}
		}
	}
	param = 0;
}



void *ActionSongMoveSelection::execute(Data *d) {
	for (auto &d: samples)
		d.sample->pos = d.pos_old + param;
	for (auto &d: notes)
		d.note->range.offset = d.pos_old + param;
	for (auto &d: markers)
		d.marker->range.offset = d.pos_old + param;
	for (auto &b: buffers)
		b.layer->buffers[b.index].offset = b.pos_old + param;
	for (auto *t: tracks)
		t->notify();
	return nullptr;
}



void ActionSongMoveSelection::abort(Data *d) {
	undo(d);
}



void ActionSongMoveSelection::undo(Data *d) {
	for (auto &d: samples)
		d.sample->pos = d.pos_old;
	for (auto &d: notes)
		d.note->range.offset = d.pos_old;
	for (auto &d: markers)
		d.marker->range.offset = d.pos_old;
	for (auto &b: buffers)
		b.layer->buffers[b.index].offset = b.pos_old;
	for (auto *t: tracks)
		t->notify();
}



void ActionSongMoveSelection::set_param_and_notify(Data *d, int _param) {
	param = _param;
	execute(d);
	d->notify();
}

void ActionSongMoveSelection::abort_and_notify(Data *d) {
	abort(d);
	d->notify();
}

bool ActionSongMoveSelection::is_trivial() {
	return (param == 0);
}



