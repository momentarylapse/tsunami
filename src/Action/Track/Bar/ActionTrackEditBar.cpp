/*
 * ActionTrackEditBar.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionTrackEditBar.h"
#include "../../../Data/Track.h"
#include <assert.h>

ActionTrackEditBar::ActionTrackEditBar(Track *t, int _index, BarPattern &_bar, bool _affect_midi)
{
	track_no = get_track_index(t);
	index = _index;
	bar = _bar;
	affect_midi = _affect_midi;
}

ActionTrackEditBar::~ActionTrackEditBar()
{
}

void *ActionTrackEditBar::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);
	assert(t);
	assert(index >= 0);
	assert(index < t->bars.num);

	if (affect_midi){
		int pos = t->barOffset(index);
		int l0 = t->bars[index].length;
		foreach(Track *tt, a->tracks){
			if (tt->type != tt->TYPE_MIDI)
				continue;
			foreachi(MidiNote &n, tt->midi, j){
				// note start
				if (n.range.start() > pos + l0)
					// after bar
					n.range.set_start(n.range.start() + bar.length - l0);
				else if (n.range.start() > pos)
					// inside bar
					n.range.set_start(pos + (float)(n.range.start() - pos) * (float)bar.length / (float)l0);
				// note end
				if (n.range.end() > pos + l0)
					// after bar
					n.range.set_end(n.range.end() + bar.length - l0);
				else if (n.range.end() > pos)
					// inside bar
					n.range.set_end(pos + (float)(n.range.end() - pos) * (float)bar.length / (float)l0);
			}
		}
	}

	BarPattern temp = bar;
	bar = t->bars[index];
	t->bars[index] = temp;
	t->notify();

	return NULL;
}

void ActionTrackEditBar::undo(Data *d)
{
	execute(d);
}

