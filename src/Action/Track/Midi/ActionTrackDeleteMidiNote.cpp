/*
 * ActionTrackDeleteMidiNote.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ActionTrackDeleteMidiNote.h"
#include "../../../Data/Track.h"

ActionTrackDeleteMidiNote::ActionTrackDeleteMidiNote(TrackLayer* l, int _index)
{
	layer = l;
	index = _index;
	note = NULL;
}

ActionTrackDeleteMidiNote::~ActionTrackDeleteMidiNote()
{
	if (note)
		delete(note);
}

void* ActionTrackDeleteMidiNote::execute(Data* d)
{
	note = layer->midi[index];
	layer->midi.erase(index);
	return NULL;
}

void ActionTrackDeleteMidiNote::undo(Data* d)
{
	layer->midi.insert(note, index);
	note = NULL;
}
