/*
 * ActionTrackDeleteMidiNote.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ActionTrackDeleteMidiNote.h"
#include "../../../Data/Track.h"

ActionTrackDeleteMidiNote::ActionTrackDeleteMidiNote(Track* t, int _index)
{
	track_no = get_track_index(t);
	index = _index;
	note = NULL;
}

void* ActionTrackDeleteMidiNote::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	note = t->midi[index];
	t->midi.erase(index);
	return NULL;
}

void ActionTrackDeleteMidiNote::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	t->midi.insert(note, index);
}
