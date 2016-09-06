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

void Selection::clear()
{
	type = TYPE_NONE;
	track = NULL;
	vtrack = NULL;
	sample = NULL;
	index = 0;
	pos = 0;
	sample_offset = 0;
	show_track_controls = NULL;
	pitch = -1;
	clef_position = -1;
	modifier = MODIFIER_UNKNOWN;
}

bool hover_changed(Selection &hover, Selection &hover_old)
{
	return (hover.type != hover_old.type)
			or (hover.sample != hover_old.sample)
			or (hover.show_track_controls != hover_old.show_track_controls)
			or (hover.index != hover_old.index)
			or (hover.pitch != hover_old.pitch)
			or (hover.clef_position != hover_old.clef_position);
}

