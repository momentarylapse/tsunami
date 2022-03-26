/*
 * ActionTrackInsertMidi.cpp
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#include "ActionTrackInsertMidi.h"
#include "../../../data/TrackLayer.h"

ActionTrackInsertMidi::ActionTrackInsertMidi(TrackLayer *l, int _offset, const MidiNoteBuffer &_midi) {
	layer = l;;
	offset = _offset;
	midi = _midi;
	for (MidiNote *n: weak(midi))
		n->range.offset += offset;
	midi.sort();
	applied = false;
}


void *ActionTrackInsertMidi::execute(Data *d) {
	inserted_at.clear();

	foreachb(MidiNote *n, weak(midi)) {
		int index = layer->midi.num;
		for (int i=0;i<layer->midi.num;i++)
			if (n->range.offset < layer->midi[i]->range.offset) {
				index = i;
				break;
			}
		layer->midi.insert(n, index);
		inserted_at.add(index);
	}
	applied = true;
	layer->notify(layer->MESSAGE_CHANGE);

	return nullptr;
}

void ActionTrackInsertMidi::undo(Data *d) {
	foreachb(int i, inserted_at)
		layer->midi.erase(i);

	applied = false;
	layer->notify(layer->MESSAGE_CHANGE);
}

