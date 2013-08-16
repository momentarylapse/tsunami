/*
 * ActionTrackDeleteMidiNote.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ActionTrackDeleteMidiNote.h"

ActionTrackDeleteMidiNote::ActionTrackDeleteMidiNote(Track* t, int _index)
{
	track_no = get_track_index(t);
	index = _index;
}

ActionTrackDeleteMidiNote::~ActionTrackDeleteMidiNote()
{
}

void* ActionTrackDeleteMidiNote::execute(Data* d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	note = t->midi[index];
	t->midi.erase(index);
	return NULL;
}

void ActionTrackDeleteMidiNote::undo(Data* d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	t->midi.insert(note, index);
}
