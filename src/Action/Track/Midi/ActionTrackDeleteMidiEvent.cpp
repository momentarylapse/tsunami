/*
 * ActionTrackDeleteMidiEvent.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ActionTrackDeleteMidiEvent.h"

ActionTrackDeleteMidiEvent::ActionTrackDeleteMidiEvent(Track* t, int _index)
{
	track_no = get_track_index(t);
	index = _index;
}

void* ActionTrackDeleteMidiEvent::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	event = t->midi[index];
	t->midi.erase(index);
	return NULL;
}

void ActionTrackDeleteMidiEvent::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	t->midi.insert(event, index);
}
