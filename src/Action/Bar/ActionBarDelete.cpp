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

#include "../../Data/Song.h"
#include "../../Data/SongSelection.h"
#include "../../Rhythm/Bar.h"
#include <assert.h>

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
		SongSelection sel = SongSelection::from_range(s, r, SongSelection::Mask::MIDI_NOTES | SongSelection::Mask::SAMPLES);

		for (Track *t: s->tracks){
			foreachi (TrackMarker *m, t->markers, i){
				if (r.covers(m->range)){
					// cover
					addSubAction(new ActionTrackDeleteMarker(t, i), d);
				}else if (r.is_inside(m->range.start())){
					// cover start
					sel.set(m, false);
					addSubAction(new ActionTrackEditMarker(t, i, Range(r.offset, m->range.end() - r.end()), m->text), d);
				}else if (r.is_inside(m->range.end())){
					// cover end
					sel.set(m, false);
					addSubAction(new ActionTrackEditMarker(t, i, Range(m->range.offset, r.offset - m->range.offset), m->text), d);
				}
			}
		}

		addSubAction(new ActionSongDeleteSelection(-1, sel, true), d);

		addSubAction(new Action__ShiftData(r.end(), - r.length), d);
	}
}
