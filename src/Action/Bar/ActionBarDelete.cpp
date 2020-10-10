/*
 * ActionSongBarDelete.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionBarDelete.h"

#include "../Song/ActionSongDeleteSelection.h"
#include "../Track/Marker/ActionTrackEditMarker.h"
#include "../Track/Marker/ActionTrackDeleteMarker.h"
#include "../Track/Midi/ActionTrackEditMidiNote.h"
#include "../Track/Midi/ActionTrackDeleteMidiNote.h"

#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/TrackMarker.h"
#include "../../Data/SongSelection.h"
#include <assert.h>

#include "../../Data/Rhythm/Bar.h"
#include "Action__ShiftData.h"
#include "ActionBar__Delete.h"

ActionBarDelete::ActionBarDelete(int _index, bool _affect_data) {
	index = _index;
	affect_data = _affect_data;
}

void ActionBarDelete::build(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0 and index < s->bars.num);

	Range r = Range(s->bar_offset(index), s->bars[index]->length);
	add_sub_action(new ActionBar__Delete(index), d);

	if (affect_data) {
		SongSelection sel = SongSelection::from_range(s, r).filter(SongSelection::Mask::SAMPLES);

		for (auto t: weak(s->tracks)) {
			
			for (auto l: weak(t->layers)) {
				foreachib (auto m, weak(l->markers), i) {
					if (r.covers(m->range)) {
						// cover
						add_sub_action(new ActionTrackDeleteMarker(l, i), d);
					} else if (r.is_inside(m->range.start())) {
						// cover start
						add_sub_action(new ActionTrackEditMarker(l, m, Range(r.offset, m->range.end() - r.end()), m->text), d);
					} else if (r.is_inside(m->range.end())) {
						// cover end
						add_sub_action(new ActionTrackEditMarker(l, m, Range(m->range.offset, r.offset - m->range.offset), m->text), d);
					} else if (m->range.covers(r)) {
						// cut out part
						add_sub_action(new ActionTrackEditMarker(l, m, Range(m->range.offset, m->range.length - r.length), m->text), d);
					}
				}

				foreachib (auto m, weak(l->midi), i) {
					if (r.covers(m->range)) {
						// cover
						add_sub_action(new ActionTrackDeleteMidiNote(l, i), d);
					} else if (r.is_inside(m->range.start())) {
						// cover start
						add_sub_action(new ActionTrackEditMidiNote(m, Range(r.offset, m->range.end() - r.end()), m->pitch, m->volume, m->stringno, m->flags), d);
					} else if (r.is_inside(m->range.end())) {
						// cover end
						add_sub_action(new ActionTrackEditMidiNote(m, Range(m->range.offset, r.offset - m->range.offset), m->pitch, m->volume, m->stringno, m->flags), d);
					} else if (m->range.covers(r)) {
						// cut out part
						add_sub_action(new ActionTrackEditMidiNote(m, Range(m->range.offset, m->range.length - r.length), m->pitch, m->volume, m->stringno, m->flags), d);
					}
				}
			}
		}

		add_sub_action(new ActionSongDeleteSelection(sel), d);

		add_sub_action(new Action__ShiftData(r.end(), - r.length, Bar::EditMode::STRETCH), d);
	}
}
