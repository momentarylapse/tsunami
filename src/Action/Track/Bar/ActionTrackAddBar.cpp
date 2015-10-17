/*
 * ActionTrackAddBar.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackAddBar.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionTrackAddBar::ActionTrackAddBar(Track *t, int _index, BarPattern &_bar, bool _affect_midi)
{
	track_no = get_track_index(t);
	index = _index;
	bar = _bar;
	affect_midi = _affect_midi;
}

ActionTrackAddBar::~ActionTrackAddBar()
{
}

void *ActionTrackAddBar::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);
	assert(index >= 0);
	assert(index <= t->bars.num);

	if (affect_midi){
		int pos0 = t->barOffset(index);
		foreach(Track *tt, a->tracks){
			if (tt->type != tt->TYPE_MIDI)
				continue;
			foreachi(MidiNote &n, tt->midi, j){
				if (n.range.offset >= pos0)
					n.range.offset += bar.length;
			}
		}
	}

	t->bars.insert(bar, index);
	t->notify();

	return NULL;
}

void ActionTrackAddBar::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);

	Track *t = a->get_track(track_no);

	if (affect_midi){
		int pos0 = t->barOffset(index);
		foreach(Track *tt, a->tracks){
			if (tt->type != tt->TYPE_MIDI)
				continue;
			foreachi(MidiNote &n, tt->midi, j){
				if (n.range.offset >= pos0)
					n.range.offset -= bar.length;
			}
		}
	}

	t->bars.erase(index);
	t->notify();
}

