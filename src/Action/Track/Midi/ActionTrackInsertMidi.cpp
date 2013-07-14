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
	foreach(MidiNote &n, midi)
		n.range.offset += offset;
	midi.sort();
}

ActionTrackInsertMidi::~ActionTrackInsertMidi()
{
}


void *ActionTrackInsertMidi::execute(Data *d)
{
	AudioFile *a = dynamic_cast<AudioFile*>(d);
	Track *t = a->get_track(track_no);

	inserted_at.clear();

	foreachb(MidiNote &n, midi){
		int index = 0;
		for (int i=0;i<t->midi.num;i++)
			if (n.range.offset >= t->midi[i].range.offset){
				index = i + 1;
				break;
			}
		t->midi.insert(n, index);
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

