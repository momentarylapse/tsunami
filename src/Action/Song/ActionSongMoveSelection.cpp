/*
 * ActionSongMoveSelection.cpp
 *
 *  Created on: 17.07.2018
 *      Author: michi
 */

#include "ActionSongMoveSelection.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/TrackMarker.h"
#include "../../Data/SongSelection.h"
#include "../../Data/SampleRef.h"


ActionSongMoveSelection::ActionSongMoveSelection(Song *a, const SongSelection &sel)
{
	for (Track *t: a->tracks){
		for (TrackMarker *m: t->markers)
			if (sel.has(m)){
				MarkerSaveData d;
				d.track = t;
				d.marker = m;
				d.pos_old = m->range.offset;
				markers.add(d);
			}

		for (TrackLayer *l: t->layers){
			for (MidiNote *n: l->midi)
				if (sel.has(n)){
					NoteSaveData d;
					d.note = n;
					d.pos_old = n->range.offset;
					notes.add(d);
				}
			for (SampleRef *s: l->samples)
				if (sel.has(s)){
					SampleSaveData d;
					d.track = t;
					d.sample = s;
					d.pos_old = s->pos;
					samples.add(d);
				}
		}
	}
	param = 0;
}



void *ActionSongMoveSelection::execute(Data *d)
{
	for (SampleSaveData &d: samples)
		d.sample->pos = d.pos_old + param;
	for (NoteSaveData &d: notes)
		d.note->range.offset = d.pos_old + param;
	for (MarkerSaveData &d: markers)
		d.marker->range.offset = d.pos_old + param;
	return nullptr;
}



void ActionSongMoveSelection::abort(Data *d)
{
	undo(d);
}



void ActionSongMoveSelection::undo(Data *d)
{
	for (SampleSaveData &d: samples)
		d.sample->pos = d.pos_old;
	for (NoteSaveData &d: notes)
		d.note->range.offset = d.pos_old;
	for (MarkerSaveData &d: markers)
		d.marker->range.offset = d.pos_old;
}



void ActionSongMoveSelection::set_param_and_notify(Data *d, int _param)
{
	param = _param;
	execute(d);
	d->notify();
}

void ActionSongMoveSelection::abort_and_notify(Data *d)
{
	abort(d);
	d->notify();
}

bool ActionSongMoveSelection::is_trivial()
{
	return (param == 0);
}



