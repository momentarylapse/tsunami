/*
 * ActionTrackInsertMidi.cpp
 *
 *  Created on: 23.02.2013
 *      Author: michi
 */

#include "ActionTrackInsertMidi.h"

ActionTrackInsertMidi::ActionTrackInsertMidi(Track *t, int _offset, MidiData &_midi)
{
	track_no = get_track_index(t);
	offset = _offset;
	midi = _midi;
	midi.sort();
	foreach(MidiEvent &e, midi)
		e.pos += offset;
	midi.sort();
}


void *ActionTrackInsertMidi::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	inserted_at.clear();

	foreachb(MidiEvent &e, midi){
		int index = 0;
		for (int i=0;i<t->midi.num;i++)
			if (e.pos >= t->midi[i].pos){
				index = i + 1;
				break;
			}
		t->midi.insert(e, index);
		inserted_at.add(index);
	}

	return NULL;
}

void ActionTrackInsertMidi::undo(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	foreachb(int i, inserted_at)
		t->midi.erase(i);
}

