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
			foreachi(MidiEvent &e, tt->midi, j){
				if (e.pos < pos){
				}else if (e.pos > pos + l0){
					e.pos += bar.length - l0;
				}else{
					e.pos = pos + (float)(e.pos - pos) * (float)bar.length / (float)l0;
				}
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

