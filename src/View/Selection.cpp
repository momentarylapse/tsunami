/*
 * Selection.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "Selection.h"
#include "../Midi/MidiData.h"


Selection::Selection()
{
	clear();
}

bool Selection::allow_auto_scroll() const
{
	return (type == TYPE_SELECTION_END) or (type == TYPE_SAMPLE) or (type == TYPE_PLAYBACK);
}

bool Selection::is_in(int _type) const
{
	if (type == _type)
		return true;
	if (_type == TYPE_TRACK_HEADER)
		return (type == TYPE_TRACK_BUTTON_MUTE) or (type == TYPE_TRACK_BUTTON_SOLO) or (type == TYPE_TRACK_BUTTON_EDIT) or (type == TYPE_TRACK_BUTTON_CURVE) or (type == TYPE_TRACK_BUTTON_FX);
	if (_type == TYPE_TRACK)
		return (track != NULL);
	return false;
}

void Selection::clear()
{
	type = TYPE_NONE;
	track = NULL;
	vtrack = NULL;
	sample = NULL;
	note = NULL;
	bar = NULL;
	index = 0;
	pos = 0;
	range = Range::EMPTY;
	y0 = y1 = 0;
	sample_offset = 0;
	pitch = -1;
	clef_position = -1;
	modifier = MODIFIER_UNKNOWN;
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

