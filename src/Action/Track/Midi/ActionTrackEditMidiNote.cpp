/*
 * ActionTrackEditMidiNote.cpp
 *
 *  Created on: 16.03.2018
 *      Author: michi
 */

#include "ActionTrackEditMidiNote.h"
#include "../../../Data/Track.h"

ActionTrackEditMidiNote::ActionTrackEditMidiNote(MidiNote *n, const Range &range, float _pitch, float _volume, int _stringno, int _flags) {
	note = n;
	note2 = new MidiNote(range, _pitch, _volume);
	note2->stringno = _stringno;
	note2->flags = _flags;
	note2->clef_position = -1;
	note2->modifier = NoteModifier::UNKNOWN;
}

void* ActionTrackEditMidiNote::execute(Data* d) {
	std::swap(note->pitch, note2->pitch);
	std::swap(note->volume, note2->volume);
	std::swap(note->range, note2->range);
	std::swap(note->stringno, note2->stringno);
	std::swap(note->flags, note2->flags);
	std::swap(note->clef_position, note2->clef_position);
	std::swap(note->modifier, note2->modifier);
	return nullptr;
}

void ActionTrackEditMidiNote::undo(Data* d) {
	execute(d);
}

