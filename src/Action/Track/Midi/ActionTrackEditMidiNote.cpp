/*
 * ActionTrackEditMidiNote.cpp
 *
 *  Created on: 16.03.2018
 *      Author: michi
 */

#include "ActionTrackEditMidiNote.h"
#include "../../../Data/Track.h"

ActionTrackEditMidiNote::ActionTrackEditMidiNote(MidiNote *n, const Range &range, float _pitch, float _volume)
{
	note = n;
	offset = range.offset;
	length = range.length;
	volume = _volume;
	pitch = _pitch;
}

void* ActionTrackEditMidiNote::execute(Data* d)
{
	std::swap(note->pitch, pitch);
	std::swap(note->volume, volume);
	std::swap(note->range.offset, offset);
	std::swap(note->range.length, length);

	return nullptr;
}

void ActionTrackEditMidiNote::undo(Data* d)
{
	execute(d);
}

