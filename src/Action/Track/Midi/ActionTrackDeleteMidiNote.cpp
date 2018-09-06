/*
 * ActionTrackDeleteMidiNote.cpp
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#include "ActionTrackDeleteMidiNote.h"
#include "../../../Data/TrackLayer.h"

ActionTrackDeleteMidiNote::ActionTrackDeleteMidiNote(TrackLayer* l, int _index)
{
	layer = l;
	index = _index;
	note = nullptr;
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
	return nullptr;
}

void ActionTrackDeleteMidiNote::undo(Data* d)
{
	layer->midi.insert(note, index);
	note = nullptr;
}
