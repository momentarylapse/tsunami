/*
 * ActionTrackAddMidiNote.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ActionTrackAddMidiNote.h"

ActionTrackAddMidiNote::ActionTrackAddMidiNote(Track* t, const MidiNote& n)
{
	track_no = get_track_index(t);
	note = n;

	insert_index = 0;
	foreachi(MidiNote &nn, t->midi, i)
		if (note.range.offset > nn.range.offset)
			insert_index = i + 1;
}

void* ActionTrackAddMidiNote::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	t->midi.insert(note, insert_index);

	return NULL;
}

void ActionTrackAddMidiNote::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	t->midi.erase(insert_index);
}
