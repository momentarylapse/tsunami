/*
 * ActionTrackAddMidiNote.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ActionTrackAddMidiNote.h"
#include "../../../Data/TrackLayer.h"
#include "../../../Data/Track.h"


ActionTrackAddMidiNote::ActionTrackAddMidiNote(TrackLayer* l, MidiNote* n) {
	layer = l;
	note = n;

	note->stringno = l->track->instrument.make_string_valid(note->pitch, note->stringno);

	insert_index = 0;
	foreachi(MidiNote *nn, l->midi, i)
		if (note->range.offset > nn->range.offset)
			insert_index = i + 1;
}

void* ActionTrackAddMidiNote::execute(Data* d) {
	layer->midi.insert(note, insert_index);
	layer->notify(layer->MESSAGE_CHANGE);

	return note.get();
}

void ActionTrackAddMidiNote::undo(Data* d) {
	layer->midi.erase(insert_index);
	layer->notify(layer->MESSAGE_CHANGE);
}
