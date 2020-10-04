/*
 * ActionBarAdd.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionBarAdd.h"

#include <assert.h>

#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/TrackMarker.h"
#include "../../Data/Song.h"
#include "../../Data/Audio/AudioBuffer.h"
#include "../Track/Buffer/ActionTrack__SplitBuffer.h"
#include "../Track/Marker/ActionTrackEditMarker.h"
#include "../Track/Midi/ActionTrackEditMidiNote.h"
#include "Action__ShiftData.h"
#include "ActionBar__Add.h"

ActionBarAdd::ActionBarAdd(int _index, const BarPattern &_bar, int _mode) {
	index = _index;
	bar = new Bar(_bar);
	mode = _mode;
}

void ActionBarAdd::build(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	add_sub_action(new ActionBar__Add(index, bar), d);

	if (mode != Bar::EditMode::IGNORE) {
		int pos0 = s->bar_offset(index);

		for (Track *t: s->tracks)
			for (TrackLayer *l: t->layers)
				for (int i=l->buffers.num-1; i>=0; i--)
					if (l->buffers[i].range().is_more_inside(pos0))
						add_sub_action(new ActionTrack__SplitBuffer(l, i, pos0 - l->buffers[i].offset), d);

		add_sub_action(new Action__ShiftData(pos0, bar->length, mode), d);

		Range r = Range(pos0, bar->length);

		for (Track *t: s->tracks){
			for (TrackLayer *l: t->layers) {
				foreachi (TrackMarker *m, l->markers, i) {
					if (m->range.is_inside(pos0)) {
						// stretch
						add_sub_action(new ActionTrackEditMarker(l, m, Range(m->range.offset, m->range.length + r.length), m->text), d);
					}
				}

				foreachi (MidiNote *m, l->midi, i) {
					if (m->range.is_inside(pos0)) {
						// stretch
						add_sub_action(new ActionTrackEditMidiNote(m, Range(m->range.offset, m->range.length + r.length), m->pitch, m->volume, m->stringno, m->flags), d);
					}
				}
			}
		}
	}
}

