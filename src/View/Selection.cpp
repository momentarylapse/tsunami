/*
 * Selection.cpp
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#include "Selection.h"


Selection::Selection()
{
	type = Selection::TYPE_NONE;
	track = NULL;
	vtrack = NULL;
	sample = NULL;
	index = 0;
	pos = 0;
	sample_offset = 0;
	show_track_controls = NULL;
	pitch = -1;
	note_start = -1;
}

bool Selection::allowAutoScroll()
{
	return (type == Selection::TYPE_SELECTION_END) or (type == Selection::TYPE_SAMPLE) or (type == Selection::TYPE_PLAYBACK);
}

bool hover_changed(Selection &hover, Selection &hover_old)
{
	return (hover.type != hover_old.type)
			or (hover.sample != hover_old.sample)
			or (hover.show_track_controls != hover_old.show_track_controls)
			or (hover.note_start != hover_old.note_start)
			or (hover.pitch != hover_old.pitch);
}

