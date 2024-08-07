/*
 * ActionTrackDeleteMidiNote.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ActionTrackDeleteMidiNote.h"
#include "../../../data/TrackLayer.h"

namespace tsunami {

ActionTrackDeleteMidiNote::ActionTrackDeleteMidiNote(TrackLayer *l, int _index) {
	layer = l;
	index = _index;
	note = layer->midi[index];
}

void* ActionTrackDeleteMidiNote::execute(Data* d) {
	layer->midi.erase(index);
	layer->out_changed.notify();
	return nullptr;
}

void ActionTrackDeleteMidiNote::undo(Data* d) {
	layer->midi.insert(note, index);
	layer->out_changed.notify();
}

}
