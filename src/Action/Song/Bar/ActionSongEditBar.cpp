/*
 * ActionSongEditBar.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionSongEditBar.h"

#include "../../../Data/Track.h"
#include <assert.h>

ActionSongEditBar::ActionSongEditBar(int _index, BarPattern &_bar, bool _affect_midi)
{
	index = _index;
	bar = _bar;
	affect_midi = _affect_midi;
}

void *ActionSongEditBar::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index < s->bars.num);

	if (affect_midi){
		int pos = s->barOffset(index);
		int l0 = s->bars[index].length;
		foreach(Track *t, s->tracks){
			if (t->type != t->TYPE_MIDI)
				continue;
			foreachi(MidiNote &n, t->midi, j){
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
	bar = s->bars[index];
	s->bars[index] = temp;
	s->notify(s->MESSAGE_EDIT_BARS);

	return NULL;
}

void ActionSongEditBar::undo(Data *d)
{
	execute(d);
}

