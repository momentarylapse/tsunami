/*
 * Selection.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "Selection.h"
#include "../Data/Midi/MidiData.h"


Selection::Selection()
{
	clear();
}

bool Selection::allow_auto_scroll() const
{
	return (type == Type::SELECTION_END) or (type == Type::SAMPLE) or (type == Type::PLAYBACK) or (type == Type::MIDI_PITCH) or (type == Type::CLEF_POSITION);
}

bool Selection::is_in(Type _type) const
{
	if (type == _type)
		return true;
	if (_type == Type::TRACK_HEADER)
		return (type == Type::TRACK_BUTTON_MUTE) or (type == Type::TRACK_BUTTON_SOLO) or (type == Type::TRACK_BUTTON_EDIT) or (type == Type::TRACK_BUTTON_CURVE) or (type == Type::TRACK_BUTTON_FX);
	if (_type == Type::LAYER_HEADER)
		return (type == Type::LAYER_BUTTON_DOMINANT) or (type == Type::LAYER_BUTTON_SOLO) or (type == Type::LAYER_BUTTON_IMPLODE) or (type == Type::LAYER_BUTTON_EXPLODE);
	if (_type == Type::LAYER)
		return (layer != nullptr);
	return false;
}

void Selection::clear()
{
	type = Type::NONE;
	track = nullptr;
	vtrack = nullptr;
	layer = nullptr;
	vlayer = nullptr;
	sample = nullptr;
	note = nullptr;
	marker = nullptr;
	bar = nullptr;
	index = 0;
	pos = 0;
	range = Range::EMPTY;
	y0 = y1 = 0;
	pitch = -1;
	clef_position = -1;
	modifier = NoteModifier::UNKNOWN;
}

bool hover_changed(Selection &hover, Selection &hover_old)
{
	return (hover.type != hover_old.type)
			or (hover.sample != hover_old.sample)
			or (hover.note != hover_old.note)
			or (hover.bar != hover_old.bar)
			or (hover.index != hover_old.index)
			or (hover.pitch != hover_old.pitch)
			or (hover.clef_position != hover_old.clef_position);
}

