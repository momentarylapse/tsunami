/*
 * ActionTrackInsertMidi.cpp
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#include "ActionTrackInsertMidi.h"

ActionTrackInsertMidi::ActionTrackInsertMidi(TrackLayer *l, int _offset, const MidiNoteBuffer &_midi)
{
	layer = l;;
	offset = _offset;
	midi = _midi;
	for (MidiNote *n: midi)
		n->range.offset += offset;
	midi.sort();
	applied = false;
}

ActionTrackInsertMidi::~ActionTrackInsertMidi()
{
	if (applied)
		midi.clear();
}


void *ActionTrackInsertMidi::execute(Data *d)
{
	inserted_at.clear();

	foreachb(MidiNote *n, midi){
		int index = layer->midi.num;
		for (int i=0;i<layer->midi.num;i++)
			if (n->range.offset < layer->midi[i]->range.offset){
				index = i;
				break;
			}
		layer->midi.insert(n, index);
		inserted_at.add(index);
	}
	applied = true;

	return NULL;
}

void ActionTrackInsertMidi::undo(Data *d)
{
	foreachb(int i, inserted_at)
		layer->midi.erase(i);

	applied = false;
}

