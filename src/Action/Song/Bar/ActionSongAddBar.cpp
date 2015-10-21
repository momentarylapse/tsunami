/*
 * ActionSongAddBar.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionSongAddBar.h"

#include "../../../Data/Track.h"
#include <assert.h>

ActionSongAddBar::ActionSongAddBar(int _index, BarPattern &_bar, bool _affect_midi)
{
	index = _index;
	bar = _bar;
	affect_midi = _affect_midi;
}

void *ActionSongAddBar::execute(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0);
	assert(index <= s->bars.num);

	if (affect_midi){
		int pos0 = s->barOffset(index);
		foreach(Track *t, s->tracks){
			if (t->type != t->TYPE_MIDI)
				continue;
			foreachi(MidiNote &n, t->midi, j){
				if (n.range.offset >= pos0)
					n.range.offset += bar.length;
			}
		}
	}

	s->bars.insert(bar, index);
	s->notify(s->MESSAGE_EDIT_BARS);

	return NULL;
}

void ActionSongAddBar::undo(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);

	if (affect_midi){
		int pos0 = s->barOffset(index);
		foreach(Track *tt, s->tracks){
			if (tt->type != tt->TYPE_MIDI)
				continue;
			foreachi(MidiNote &n, tt->midi, j){
				if (n.range.offset >= pos0)
					n.range.offset -= bar.length;
			}
		}
	}

	s->bars.erase(index);
	s->notify(s->MESSAGE_EDIT_BARS);
}

