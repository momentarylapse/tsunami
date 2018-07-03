/*
 * ActionTrackAddMidiNote.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ActionTrackAddMidiNote.h"
#include "../../../Data/Track.h"

ActionTrackAddMidiNote::ActionTrackAddMidiNote(Track* t, MidiNote* n)
{
	track = t;
	note = n;

	insert_index = 0;
	foreachi(MidiNote *nn, t->midi, i)
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
	track->midi.insert(note, insert_index);
	note = NULL;

	return note;
}

void ActionTrackAddMidiNote::undo(Data* d)
{
	note = track->midi[insert_index];
	track->midi.erase(insert_index);
}
