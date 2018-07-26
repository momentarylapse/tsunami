/*
 * ActionTrackAddMidiNote.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ActionTrackAddMidiNote.h"
#include "../../../Data/Track.h"

ActionTrackAddMidiNote::ActionTrackAddMidiNote(TrackLayer* l, MidiNote* n)
{
	layer = l;
	note = n;

	insert_index = 0;
	foreachi(MidiNote *nn, l->midi, i)
		if (note->range.offset > nn->range.offset)
			insert_index = i + 1;
}

ActionTrackAddMidiNote::~ActionTrackAddMidiNote()
{
	if (note)
		delete(note);
}

void* ActionTrackAddMidiNote::execute(Data* d)
{
	layer->midi.insert(note, insert_index);
	note = nullptr;

	return note;
}

void ActionTrackAddMidiNote::undo(Data* d)
{
	note = layer->midi[insert_index];
	layer->midi.erase(insert_index);
}
