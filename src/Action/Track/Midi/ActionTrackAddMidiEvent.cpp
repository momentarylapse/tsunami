/*
 * ActionTrackAddMidiEvent.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ActionTrackAddMidiEvent.h"

ActionTrackAddMidiEvent::ActionTrackAddMidiEvent(Track* t, const MidiEvent& e)
{
	track_no = get_track_index(t);
	event = e;

	insert_index = 0;
	foreachi(MidiEvent &ee, t->midi, i)
		if (event.pos > ee.pos)
			insert_index = i + 1;
}

void* ActionTrackAddMidiEvent::execute(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	t->midi.insert(event, insert_index);

	return NULL;
}

void ActionTrackAddMidiEvent::undo(Data* d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	t->midi.erase(insert_index);
}
