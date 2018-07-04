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
#include "../../Data/SongSelection.h"
#include <assert.h>

#include "../../Data/Rhythm/Bar.h"
#include "Action__ShiftData.h"
#include "ActionBar__Delete.h"

ActionBarDelete::ActionBarDelete(int _index, bool _affect_data)
{
	index = _index;
	affect_data = _affect_data;
}

void ActionBarDelete::build(Data *d)
{
	Song *s = dynamic_cast<Song*>(d);
	assert(index >= 0 and index < s->bars.num);

	Range r = Range(s->barOffset(index), s->bars[index]->length);
	addSubAction(new ActionBar__Delete(index), d);

	if (affect_data){
		SongSelection sel = SongSelection::from_range(s, r, SongSelection::Mask::SAMPLES);

		for (Track *t: s->tracks){
			foreachi (TrackMarker *m, t->markers, i){
				if (r.covers(m->range)){
					// cover
					addSubAction(new ActionTrackDeleteMarker(t, i), d);
				}else if (r.is_inside(m->range.start())){
					// cover start
					addSubAction(new ActionTrackEditMarker(m, Range(r.offset, m->range.end() - r.end()), m->text), d);
				}else if (r.is_inside(m->range.end())){
					// cover end
					addSubAction(new ActionTrackEditMarker(m, Range(m->range.offset, r.offset - m->range.offset), m->text), d);
				}else if (m->range.covers(r)){
					// cut out part
					addSubAction(new ActionTrackEditMarker(m, Range(m->range.offset, m->range.length - r.length), m->text), d);
				}
			}

			for (TrackLayer *l: t->layers)
			foreachi (MidiNote *m, l->midi, i){
				if (r.covers(m->range)){
					// cover
					addSubAction(new ActionTrackDeleteMidiNote(l, i), d);
				}else if (r.is_inside(m->range.start())){
					// cover start
					addSubAction(new ActionTrackEditMidiNote(m, Range(r.offset, m->range.end() - r.end()), m->pitch, m->volume), d);
				}else if (r.is_inside(m->range.end())){
					// cover end
					addSubAction(new ActionTrackEditMidiNote(m, Range(m->range.offset, r.offset - m->range.offset), m->pitch, m->volume), d);
				}else if (m->range.covers(r)){
					// cut out part
					addSubAction(new ActionTrackEditMidiNote(m, Range(m->range.offset, m->range.length - r.length), m->pitch, m->volume), d);
				}
			}
		}

		addSubAction(new ActionSongDeleteSelection(sel), d);

		addSubAction(new Action__ShiftData(r.end(), - r.length, Bar::EditMode::STRETCH), d);
	}
}
