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
	foreachi(MidiNote &n, t->midi, i)
		if (note.range.offset > n.range.offset)
			insert_index = i + 1;
}

ActionTrackAddMidiNote::~ActionTrackAddMidiNote()
{
}

void* ActionTrackAddMidiNote::execute(Data* d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	t->midi.insert(note, insert_index);

	return NULL;
}

void ActionTrackAddMidiNote::undo(Data* d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	t->midi.erase(insert_index);
}
