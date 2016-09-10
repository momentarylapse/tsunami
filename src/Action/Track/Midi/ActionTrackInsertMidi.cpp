/*
 * ActionTrackInsertMidi.cpp
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#include "ActionTrackInsertMidi.h"

ActionTrackInsertMidi::ActionTrackInsertMidi(Track *t, int _offset, const MidiData &_midi)
{
	track_no = get_track_index(t);
	offset = _offset;
	midi = _midi;
	for (MidiNote *n: midi)
		n->range.offset += offset;
	midi.sort();
	applied = false;
}

ActionTrackInsertMidi::~ActionTrackInsertMidi()
{
	if (applied)
		midi.clear();
}


void *ActionTrackInsertMidi::execute(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	inserted_at.clear();

	foreachb(MidiNote *n, midi){
		int index = t->midi.num;
		for (int i=0;i<t->midi.num;i++)
			if (n->range.offset < t->midi[i]->range.offset){
				index = i;
				break;
			}
		t->midi.insert(n, index);
		inserted_at.add(index);
	}
	applied = true;

	return NULL;
}

void ActionTrackInsertMidi::undo(Data *d)
{
	Song *a = dynamic_cast<Song*>(d);
	Track *t = a->get_track(track_no);

	foreachb(int i, inserted_at)
		t->midi.erase(i);

	applied = false;
}

